/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_DEPRECATED_CLIENT_PREFERENCES_AD_PREFERENCES_INFO_H_
#define BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_DEPRECATED_CLIENT_PREFERENCES_AD_PREFERENCES_INFO_H_

#include <string>

#include "base/values.h"
#include "bat/ads/internal/deprecated/client/preferences/filtered_advertiser_info.h"
#include "bat/ads/internal/deprecated/client/preferences/filtered_category_info.h"
#include "bat/ads/internal/deprecated/client/preferences/flagged_ad_info.h"
#include "bat/ads/internal/deprecated/client/preferences/saved_ad_info.h"

namespace ads {

struct AdPreferencesInfo final {
  AdPreferencesInfo();

  AdPreferencesInfo(const AdPreferencesInfo& other);
  AdPreferencesInfo& operator=(const AdPreferencesInfo& other);

  AdPreferencesInfo(AdPreferencesInfo&& other) noexcept;
  AdPreferencesInfo& operator=(AdPreferencesInfo&& other) noexcept;

  ~AdPreferencesInfo();

  base::Value::Dict ToValue() const;
  void FromValue(const base::Value::Dict& root);

  std::string ToJson() const;
  bool FromJson(const std::string& json);

  FilteredAdvertiserList filtered_advertisers;
  FilteredCategoryList filtered_categories;
  SavedAdList saved_ads;
  FlaggedAdList flagged_ads;
};

}  // namespace ads

#endif  // BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_DEPRECATED_CLIENT_PREFERENCES_AD_PREFERENCES_INFO_H_
