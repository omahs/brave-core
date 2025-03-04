// Copyright (c) 2022 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

#include "brave/components/brave_today/browser/locales_helper.h"

#include <string>
#include <vector>

#include "base/containers/flat_map.h"
#include "base/containers/flat_set.h"
#include "base/feature_list.h"
#include "base/ranges/algorithm.h"
#include "base/strings/string_util.h"
#include "brave/components/brave_today/browser/publishers_controller.h"
#include "brave/components/brave_today/browser/urls.h"
#include "brave/components/brave_today/common/brave_news.mojom-forward.h"
#include "brave/components/brave_today/common/brave_news.mojom-shared.h"
#include "brave/components/brave_today/common/brave_news.mojom.h"
#include "brave/components/brave_today/common/features.h"
#include "third_party/abseil-cpp/absl/types/optional.h"

namespace brave_news {
namespace {
bool HasAnyLocale(const base::flat_set<std::string>& locales,
                  const mojom::Publisher* publisher) {
  return base::ranges::any_of(
      publisher->locales,
      [&locales](const auto& locale) { return locales.contains(locale); });
}

absl::optional<std::string> GetBestMissingLocale(
    const base::flat_set<std::string>& locales,
    const std::vector<mojom::Publisher*> publishers) {
  base::flat_map<std::string, uint32_t> missing_locale_counts;
  for (auto const* publisher : publishers) {
    // If this publisher is already covered by the list of locales we don't want
    // its locales to skew the list of what's missing.
    if (HasAnyLocale(locales, publisher))
      continue;

    for (const auto& locale : publisher->locales)
      missing_locale_counts[locale]++;
  }

  if (missing_locale_counts.empty())
    return {};

  return base::ranges::max_element(
             missing_locale_counts,
             [](const auto& a, const auto& b) { return a.second < b.second; })
      ->first;
}

}  // namespace

base::flat_set<std::string> GetPublisherLocales(const Publishers& publishers) {
  base::flat_set<std::string> result;
  for (const auto& [_, publisher] : publishers) {
    result.insert(publisher->locales.begin(), publisher->locales.end());
  }
  return result;
}

base::flat_set<std::string> GetMinimalLocalesSet(
    const base::flat_set<std::string>& channel_locales,
    const Publishers& publishers) {
  if (!base::FeatureList::IsEnabled(
          brave_today::features::kBraveNewsV2Feature)) {
    return {brave_today::GetV1RegionUrlPart()};
  }

  // All channel locales are part of the minimal set - we need to include all of
  // them.
  base::flat_set<std::string> result = channel_locales;

  std::vector<mojom::Publisher*> subscribed_publishers;
  for (const auto& [id, publisher] : publishers) {
    // This API is only used by the V2 news API, so we don't need to care about
    // the legacy |.enabled| property.
    // We are only interested in explicitly enabled publishers, as channel
    // enabled ones will be covered by |channel_locales|.
    if (publisher->user_enabled_status != mojom::UserEnabled::ENABLED)
      continue;
    subscribed_publishers.push_back(publisher.get());
  }

  // While there are publishers which won't be included in the feed, add a new
  // locale and recalculate what's missing.
  absl::optional<std::string> best_missing_locale;
  while ((best_missing_locale =
              GetBestMissingLocale(result, subscribed_publishers))) {
    result.insert(best_missing_locale.value());
  }

  return result;
}
}  // namespace brave_news
