/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_LEGACY_MIGRATION_REWARDS_LEGACY_REWARDS_MIGRATION_UNBLINDED_PAYMENT_TOKENS_JSON_READER_UTIL_H_
#define BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_LEGACY_MIGRATION_REWARDS_LEGACY_REWARDS_MIGRATION_UNBLINDED_PAYMENT_TOKENS_JSON_READER_UTIL_H_

#include "absl/types/optional.h"
#include "bat/ads/internal/privacy/tokens/unblinded_payment_tokens/unblinded_payment_token_info.h"

namespace base {
class Value;
}  // namespace base

namespace ads::rewards::json::reader {

absl::optional<privacy::UnblindedPaymentTokenList> ParseUnblindedPaymentTokens(
    const base::Value& value);

}  // namespace ads::rewards::json::reader

#endif  // BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_LEGACY_MIGRATION_REWARDS_LEGACY_REWARDS_MIGRATION_UNBLINDED_PAYMENT_TOKENS_JSON_READER_UTIL_H_
