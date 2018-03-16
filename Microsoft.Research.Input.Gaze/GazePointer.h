//Copyright (c) Microsoft. All rights reserved. Licensed under the MIT license.
//See LICENSE in the project root for license information.

#pragma once
#pragma warning(disable:4453)

#include "IGazeFilter.h"
#include "GazeCursor.h"

using namespace Platform::Collections;
using namespace Windows::Foundation;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Devices::HumanInterfaceDevice;
using namespace Windows::UI::Core;
using namespace Windows::Devices::Input::Preview;

namespace Shapes = Windows::UI::Xaml::Shapes;

BEGIN_NAMESPACE_GAZE_INPUT

// units in microseconds
const int DEFAULT_FIXATION_DELAY = 400000;
const int DEFAULT_DWELL_DELAY = 800000;
const int DEFAULT_REPEAT_DELAY = MAXINT;
const int DEFAULT_ENTER_EXIT_DELAY = 50000;
const int DEFAULT_MAX_HISTORY_DURATION = 3000000;
const int MAX_SINGLE_SAMPLE_DURATION = 100000;

const int GAZE_IDLE_TIME = 2500000;

public enum class GazePointerState
{
	Exit,

	// The order of the following elements is important because
	// they represent states that linearly transition to their
	// immediate successors. 
	PreEnter,
	Enter,
	Fixation,
	Dwell,
	//FixationRepeat,
	DwellRepeat
};

// assocate a particular GazePointerState with a duration
ref class GazeInvokeParams sealed
{
public:

	GazeInvokeParams()
	{
		Fixation = DEFAULT_FIXATION_DELAY;
		Dwell = DEFAULT_DWELL_DELAY;
		DwellRepeat = DEFAULT_REPEAT_DELAY;
		Enter = DEFAULT_ENTER_EXIT_DELAY;
		Exit = DEFAULT_ENTER_EXIT_DELAY;
	}

	GazeInvokeParams(GazeInvokeParams^ value)
	{
		Fixation = value->Fixation;
		Dwell = value->Dwell;
		DwellRepeat = value->DwellRepeat;
		Enter = value->Enter;
		Exit = value->Exit;
	}

	property int Fixation;
	property int Dwell;
	property int DwellRepeat;
	property int Enter;
	property int Exit;

	int Get(GazePointerState state)
	{
		switch (state)
		{
		case GazePointerState::Fixation: return Fixation;
		case GazePointerState::Dwell: return Dwell;
		case GazePointerState::DwellRepeat: return DwellRepeat;
		case GazePointerState::Enter: return Enter;
		case GazePointerState::Exit: return Exit;
		default: return 0;
		}
	}

	void Set(GazePointerState state, int value)
	{
		switch (state)
		{
		case GazePointerState::Fixation: Fixation = value; break;
		case GazePointerState::Dwell: Dwell = value; break;
		case GazePointerState::DwellRepeat: DwellRepeat = value; break;
		case GazePointerState::Enter: Enter = value; break;
		case GazePointerState::Exit: Exit = value; break;
		}
	}
};

ref struct GazeHistoryItem
{
	property UIElement^ HitTarget;
	property int64 Timestamp;
	property int Duration;
};

ref struct GazeTargetItem sealed
{
	property int ElapsedTime;
	property int NextStateTime;
	property int64 LastTimestamp;
	property GazePointerState ElementState;
	property UIElement^ TargetElement;
	// used to keep track of when the next DwellRepeat event is to be fired
	property int NextDwellRepeatTime;

	GazeTargetItem(UIElement^ target)
	{
		TargetElement = target;
	}

	void Reset(int nextStateTime, int nextRepeatTime)
	{
		ElementState = GazePointerState::PreEnter;
		ElapsedTime = 0;
		NextStateTime = nextStateTime;
		NextDwellRepeatTime = nextRepeatTime;
	}
};

public ref struct GazePointerEventArgs sealed
{
	property UIElement^ HitTarget;
	property GazePointerState PointerState;
	property int ElapsedTime;

	GazePointerEventArgs(UIElement^ target, GazePointerState state, int elapsedTime)
	{
		HitTarget = target;
		PointerState = state;
		ElapsedTime = elapsedTime;
	}
};

ref class GazePointer;
public delegate void GazePointerEvent(GazePointer^ sender, GazePointerEventArgs^ ea);
public delegate void GazeInputEvent(GazePointer^ sender, GazeEventArgs^ ea);

public delegate bool GazeIsInvokableDelegate(UIElement^ target);
public delegate void GazeInvokeTargetDelegate(UIElement^ target);

public ref class GazePointer sealed
{
public:
	GazePointer(UIElement^ root);
	virtual ~GazePointer();

	property GazeIsInvokableDelegate^ IsInvokableImpl
	{
		GazeIsInvokableDelegate^ get()
		{
			return _isInvokableImpl;
		}
		void set(GazeIsInvokableDelegate^ value)
		{
			_isInvokableImpl = value;
		}
	}

	property GazeInvokeTargetDelegate^ InvokeTargetImpl
	{
		GazeInvokeTargetDelegate^ get()
		{
			return _invokeTargetImpl;
		}
		void set(GazeInvokeTargetDelegate^ value)
		{
			_invokeTargetImpl = value;
		}
	}

	void InvokeTarget(UIElement^ target);
	void Reset();
	void SetElementStateDelay(UIElement ^element, GazePointerState pointerState, int stateDelay);
	int GetElementStateDelay(UIElement^ element, GazePointerState pointerState);

	event GazePointerEvent^ OnGazePointerEvent;
	event GazeInputEvent^ OnGazeInputEvent;

	// Provide a configurable delay for when the EyesOffDelay event is fired
	// GOTCHA: this value requires that _eyesOffTimer is instantiated so that it
	// can update the timer interval 
	property int64 EyesOffDelay
	{
		int64 get() { return _eyesOffDelay; }
		void set(int64 value)
		{
			_eyesOffDelay = value;

			// convert GAZE_IDLE_TIME units (microseconds) to 100-nanosecond units used
			// by TimeSpan struct
			_eyesOffTimer->Interval = TimeSpan { EyesOffDelay * 10 };
		}
	}

	// Pluggable filter for eye tracking sample data. This defaults to being set to the
	// NullFilter which performs no filtering of input samples.
	property IGazeFilter^ Filter;

	property bool IsCursorVisible
	{
		bool get() { return _gazeCursor->IsCursorVisible; }
		void set(bool value) { _gazeCursor->IsCursorVisible = value; }
	}

	property int CursorRadius
	{
		int get() { return _gazeCursor->CursorRadius; }
		void set(int value) { _gazeCursor->CursorRadius = value; }
	}

	property bool InputEventForwardingEnabled;

private:
	void    InitializeHistogram();
	void    InitializeGazeInputSource();

	GazeInvokeParams^   GetReadGazeInvokeParams(UIElement^ target);
	GazeInvokeParams^   GetWriteGazeInvokeParams(UIElement^ target);
	GazeTargetItem^     GetOrCreateGazeTargetItem(UIElement^ target);
	GazeTargetItem^     GetGazeTargetItem(UIElement^ target);
	UIElement^          GetHitTarget(Point gazePoint);
	UIElement^          ResolveHitTarget(Point gazePoint, long long timestamp);

	bool    IsInvokable(UIElement^ target);

	void    CheckIfExiting(long long curTimestamp);
	void    GotoState(UIElement^ control, GazePointerState state);
	void	RaiseGazePointerEvent(UIElement^ target, GazePointerState state, int elapsedTime);

	void OnGazeMoved(
		GazeInputSourcePreview^ provider,
		GazeMovedPreviewEventArgs^ args);

	void ProcessGazePoint(GazePointPreview^ gazePointPreview);

	void    OnEyesOff(Object ^sender, Object ^ea);


private:
	UIElement^        _rootElement;

	int64            _eyesOffDelay;

	GazeCursor^      _gazeCursor;
	DispatcherTimer^ _eyesOffTimer;

	// _offScreenElement is a pseudo-element that represents the area outside
	// the screen so we can track how long the user has been looking outside
	// the screen and appropriately trigger the EyesOff event
	Control^          _offScreenElement;
	GazeInvokeParams^  _defaultInvokeParams;

	// The value is the total time that FrameworkElement has been gazed at
	Vector<GazeTargetItem^>^        _activeHitTargetTimes;

	// A vector to track the history of observed gaze targets
	Vector<GazeHistoryItem^>^       _gazeHistory;
	int64                             _maxHistoryTime;

	// Used to determine if exit events need to be fired by adding GAZE_IDLE_TIME to the last 
	// saved timestamp
	long long                       _lastTimestamp;

	GazeInputSourcePreview^          _gazeInputSource;
	EventRegistrationToken          _gazeMovedToken;
	CoreDispatcher^                 _coreDispatcher;
	GazeIsInvokableDelegate^        _isInvokableImpl;
	GazeInvokeTargetDelegate^       _invokeTargetImpl;
};

END_NAMESPACE_GAZE_INPUT