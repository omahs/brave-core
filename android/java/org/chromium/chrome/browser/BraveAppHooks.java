/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.chromium.chrome.browser;

import org.chromium.chrome.browser.feedback.HelpAndFeedbackLauncher;
import org.chromium.chrome.browser.help.BraveHelpAndFeedbackLauncher;

public class BraveAppHooks extends AppHooksImpl {
    @Override
    public HelpAndFeedbackLauncher createHelpAndFeedbackLauncher() {
        return new BraveHelpAndFeedbackLauncher();
    }
}
