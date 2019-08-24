/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_shields/browser/ad_block_regional_service_manager.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(AdBlockRegionalServiceTest, UserModelLanguages) {
  std::vector<std::string> languages({ "fr", "fR", "fr-FR", "fr-ca" });
  std::for_each(languages.begin(), languages.end(),
      [](const std::string& language) {
    EXPECT_TRUE(brave_shields::AdBlockRegionalServiceManager::IsSupportedLocale(
        language));
  });
}
