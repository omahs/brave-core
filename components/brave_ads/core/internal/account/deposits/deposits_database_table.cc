/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/core/internal/account/deposits/deposits_database_table.h"

#include <utility>

#include "base/check.h"
#include "base/functional/bind.h"
#include "base/strings/string_util.h"
#include "brave/components/brave_ads/common/interfaces/brave_ads.mojom.h"
#include "brave/components/brave_ads/core/internal/ads_client_helper.h"
#include "brave/components/brave_ads/core/internal/common/database/database_bind_util.h"
#include "brave/components/brave_ads/core/internal/common/database/database_column_util.h"
#include "brave/components/brave_ads/core/internal/common/database/database_transaction_util.h"
#include "brave/components/brave_ads/core/internal/common/logging_util.h"

namespace brave_ads::database::table {

namespace {

constexpr char kTableName[] = "deposits";

void BindRecords(mojom::DBCommandInfo* command) {
  DCHECK(command);

  command->record_bindings = {
      mojom::DBCommandInfo::RecordBindingType::
          STRING_TYPE,  // creative_instance_id
      mojom::DBCommandInfo::RecordBindingType::DOUBLE_TYPE,  // value
      mojom::DBCommandInfo::RecordBindingType::DOUBLE_TYPE   // expire_at
  };
}

size_t BindParameters(mojom::DBCommandInfo* command,
                      const CreativeAdList& creative_ads) {
  DCHECK(command);

  size_t count = 0;

  int index = 0;
  for (const auto& creative_ad : creative_ads) {
    BindString(command, index++, creative_ad.creative_instance_id);
    BindDouble(command, index++, creative_ad.value);
    BindDouble(command, index++, creative_ad.end_at.ToDoubleT());

    count++;
  }

  return count;
}

void BindParameters(mojom::DBCommandInfo* command, const DepositInfo& deposit) {
  DCHECK(command);
  DCHECK(deposit.IsValid());

  BindString(command, 0, deposit.creative_instance_id);
  BindDouble(command, 1, deposit.value);
  BindDouble(command, 2, deposit.expire_at.ToDoubleT());
}

DepositInfo GetFromRecord(mojom::DBRecordInfo* record) {
  DCHECK(record);

  DepositInfo deposit;

  deposit.creative_instance_id = ColumnString(record, 0);
  deposit.value = ColumnDouble(record, 1);
  deposit.expire_at = base::Time::FromDoubleT(ColumnDouble(record, 2));

  return deposit;
}

void OnGetForCreativeInstanceId(
    const std::string& /*creative_instance_id*/,
    GetDepositsCallback callback,
    mojom::DBCommandResponseInfoPtr command_response) {
  if (!command_response ||
      command_response->status !=
          mojom::DBCommandResponseInfo::StatusType::RESPONSE_OK) {
    BLOG(0, "Failed to get deposit value");
    return std::move(callback).Run(/*success*/ false,
                                   /*deposit*/ absl::nullopt);
  }

  if (command_response->result->get_records().empty()) {
    return std::move(callback).Run(/*success*/ true, /*deposit*/ absl::nullopt);
  }

  const mojom::DBRecordInfoPtr record =
      std::move(command_response->result->get_records().front());
  DepositInfo deposit = GetFromRecord(&*record);

  std::move(callback).Run(/*success*/ true, std::move(deposit));
}

void MigrateToV24(mojom::DBTransactionInfo* transaction) {
  DCHECK(transaction);

  mojom::DBCommandInfoPtr command = mojom::DBCommandInfo::New();
  command->type = mojom::DBCommandInfo::Type::EXECUTE;
  command->sql =
      "CREATE TABLE IF NOT EXISTS deposits "
      "(creative_instance_id TEXT NOT NULL, "
      "value DOUBLE NOT NULL, "
      "expire_at TIMESTAMP NOT NULL, "
      "PRIMARY KEY (creative_instance_id), "
      "UNIQUE(creative_instance_id) ON CONFLICT REPLACE);";
  transaction->commands.push_back(std::move(command));
}

}  // namespace

void Deposits::Save(const DepositInfo& deposit, ResultCallback callback) {
  if (!deposit.IsValid()) {
    return std::move(callback).Run(/*success*/ false);
  }

  mojom::DBTransactionInfoPtr transaction = mojom::DBTransactionInfo::New();

  InsertOrUpdate(&*transaction, deposit);

  RunTransaction(std::move(transaction), std::move(callback));
}

void Deposits::InsertOrUpdate(mojom::DBTransactionInfo* transaction,
                              const CreativeAdList& creative_ads) {
  DCHECK(transaction);

  if (creative_ads.empty()) {
    return;
  }

  mojom::DBCommandInfoPtr command = mojom::DBCommandInfo::New();
  command->type = mojom::DBCommandInfo::Type::RUN;
  command->sql = BuildInsertOrUpdateSql(&*command, creative_ads);
  transaction->commands.push_back(std::move(command));
}

void Deposits::InsertOrUpdate(mojom::DBTransactionInfo* transaction,
                              const DepositInfo& deposit) {
  DCHECK(transaction);
  DCHECK(deposit.IsValid());

  mojom::DBCommandInfoPtr command = mojom::DBCommandInfo::New();
  command->type = mojom::DBCommandInfo::Type::RUN;
  command->sql = BuildInsertOrUpdateSql(&*command, deposit);
  transaction->commands.push_back(std::move(command));
}

void Deposits::GetForCreativeInstanceId(const std::string& creative_instance_id,
                                        GetDepositsCallback callback) const {
  if (creative_instance_id.empty()) {
    return std::move(callback).Run(/*success*/ false,
                                   /*deposit*/ absl::nullopt);
  }

  mojom::DBTransactionInfoPtr transaction = mojom::DBTransactionInfo::New();
  mojom::DBCommandInfoPtr command = mojom::DBCommandInfo::New();
  command->type = mojom::DBCommandInfo::Type::READ;
  command->sql = base::ReplaceStringPlaceholders(
      "SELECT creative_instance_id, value, expire_at FROM $1 AS rv WHERE "
      "rv.creative_instance_id = '$2';",
      {GetTableName(), creative_instance_id}, nullptr);
  BindRecords(&*command);
  transaction->commands.push_back(std::move(command));

  AdsClientHelper::GetInstance()->RunDBTransaction(
      std::move(transaction),
      base::BindOnce(&OnGetForCreativeInstanceId, creative_instance_id,
                     std::move(callback)));
}

void Deposits::PurgeExpired(ResultCallback callback) const {
  mojom::DBTransactionInfoPtr transaction = mojom::DBTransactionInfo::New();

  mojom::DBCommandInfoPtr command = mojom::DBCommandInfo::New();
  command->type = mojom::DBCommandInfo::Type::EXECUTE;
  command->sql = base::ReplaceStringPlaceholders(
      "DELETE FROM $1 WHERE DATETIME('now') >= "
      "DATETIME(expire_at, 'unixepoch');",
      {GetTableName()}, nullptr);
  transaction->commands.push_back(std::move(command));

  RunTransaction(std::move(transaction), std::move(callback));
}

std::string Deposits::GetTableName() const {
  return kTableName;
}

void Deposits::Migrate(mojom::DBTransactionInfo* transaction,
                       const int to_version) {
  DCHECK(transaction);

  switch (to_version) {
    case 24: {
      MigrateToV24(transaction);
      break;
    }

    default: {
      break;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

std::string Deposits::BuildInsertOrUpdateSql(
    mojom::DBCommandInfo* command,
    const CreativeAdList& creative_ads) const {
  DCHECK(command);

  const size_t binded_parameters_count = BindParameters(command, creative_ads);

  return base::ReplaceStringPlaceholders(
      "INSERT OR REPLACE INTO $1 (creative_instance_id, value, expire_at) "
      "VALUES $2;",
      {GetTableName(), BuildBindingParameterPlaceholders(
                           /*parameters_count*/ 3, binded_parameters_count)},
      nullptr);
}

std::string Deposits::BuildInsertOrUpdateSql(mojom::DBCommandInfo* command,
                                             const DepositInfo& deposit) const {
  DCHECK(command);
  DCHECK(deposit.IsValid());

  BindParameters(command, deposit);

  return base::ReplaceStringPlaceholders(
      "INSERT OR REPLACE INTO $1 (creative_instance_id, value, expire_at) "
      "VALUES $2;",
      {GetTableName(),
       BuildBindingParameterPlaceholders(/*parameters_count*/ 3,
                                         /*binded_parameters_count*/ 1)},
      nullptr);
}

}  // namespace brave_ads::database::table
