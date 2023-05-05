/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/core/internal/ads/serving/eligible_ads/pipelines/inline_content_ads/eligible_inline_content_ads_base.h"

#include "brave/components/brave_ads/core/internal/geographic/subdivision_targeting/subdivision_targeting.h"
#include "brave/components/brave_ads/core/internal/resources/behavioral/anti_targeting/anti_targeting_resource.h"

namespace brave_ads {

EligibleInlineContentAdsBase::EligibleInlineContentAdsBase(
    const SubdivisionTargeting& subdivision_targeting,
    const AntiTargetingResource& anti_targeting_resource)
    : subdivision_targeting_(subdivision_targeting),
      anti_targeting_resource_(anti_targeting_resource) {}

EligibleInlineContentAdsBase::~EligibleInlineContentAdsBase() = default;

}  // namespace brave_ads
