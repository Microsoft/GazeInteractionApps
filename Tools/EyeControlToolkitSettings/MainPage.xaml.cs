﻿//Copyright (c) Microsoft. All rights reserved. Licensed under the MIT license.
//See LICENSE in the project root for license information.

using Microsoft.Toolkit.Uwp.Input.GazeInteraction;
using Windows.Foundation.Collections;
using Windows.Storage;
using Windows.UI.Xaml.Controls;

namespace EyeControlToolkitSettings
{
    public sealed partial class MainPage : Page
    {
        public GazeSettings GazeSettings = new GazeSettings();

        public MainPage()
        {
            InitializeComponent();

            var localSettings = new ValueSet();
            GazeSettings.ValueSetFromLocalSettings(localSettings);

            var gazePointer = GazeInput.GetGazePointer(this);
            gazePointer.LoadSettings(localSettings);
        }

        #region Button Handlers
        private void GazePointerFixationDelay_NudgeDown_Click(object sender, Windows.UI.Xaml.RoutedEventArgs e)
        {
            GazeSettings.GazePointerFixationDelay -= 50000;
        }

        private void GazePointerFixationDelay_NudgeUp_Click(object sender, Windows.UI.Xaml.RoutedEventArgs e)
        {
            GazeSettings.GazePointerFixationDelay += 50000;
        }

        private void GazePointerDwellDelay_NudgeDown_Click(object sender, Windows.UI.Xaml.RoutedEventArgs e)
        {
            GazeSettings.GazePointerDwellDelay -= 50000;
        }

        private void GazePointerDwellDelay_NudgeUp_Click(object sender, Windows.UI.Xaml.RoutedEventArgs e)
        {
            GazeSettings.GazePointerDwellDelay -= 50000;
        }

        private void GazePointerRepeatDelay_NudgeDown_Click(object sender, Windows.UI.Xaml.RoutedEventArgs e)
        {

        }

        private void GazePointerRepeatDelay_NudgeUp_Click(object sender, Windows.UI.Xaml.RoutedEventArgs e)
        {

        }

        private void GazePointerGazeIdleTime_NudgeDown_Click(object sender, Windows.UI.Xaml.RoutedEventArgs e)
        {
            GazeSettings.GazePointerGazeIdleTime -= 10000;
        }

        private void GazePointerGazeIdleTime_NudgeUp_Click(object sender, Windows.UI.Xaml.RoutedEventArgs e)
        {
            GazeSettings.GazePointerGazeIdleTime += 10000;
        }

        private void GazeCursorRadius_NudgeDown_Click(object sender, Windows.UI.Xaml.RoutedEventArgs e)
        {
            GazeSettings.GazeCursorRadius -= 5;
        }

        private void GazeCursorRadius_NudgeUp_Click(object sender, Windows.UI.Xaml.RoutedEventArgs e)
        {
            GazeSettings.GazeCursorRadius += 5;
        }

        private void GazeCursorVisibility_Checkbox_Click(object sender, Windows.UI.Xaml.RoutedEventArgs e)
        {
            GazeSettings.GazeCursorVisibility = GazeCursorVisibility_Checkbox.IsChecked.Value;
        }

        private void GazePointerIsSwitchEnabled_Checkbox_Click(object sender, Windows.UI.Xaml.RoutedEventArgs e)
        {
            GazeSettings.GazePointerIsSwitchEnabled = GazePointerIsSwitchEnabled_Checkbox.IsChecked.Value;
        }

        private void ResetButton_Click(object sender, Windows.UI.Xaml.RoutedEventArgs e)
        {
            ApplicationData.Current.LocalSettings.Values.Clear();
            GazeSettings.Reset();
        }
        #endregion
    }
}
