/* copyright (c) 2022 the brave authors. all rights reserved.
 * this source code form is subject to the terms of the mozilla public
 * license, v. 2.0. if a copy of the mpl was not distributed with this file,
 * you can obtain one at http://mozilla.org/mpl/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_WALLET_BROWSER_SOLANA_TRANSACTION_H_
#define BRAVE_COMPONENTS_BRAVE_WALLET_BROWSER_SOLANA_TRANSACTION_H_

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "base/gtest_prod_util.h"
#include "brave/components/brave_wallet/browser/solana_message.h"
#include "brave/components/brave_wallet/common/brave_wallet.mojom.h"
#include "third_party/abseil-cpp/absl/types/optional.h"

namespace base {
class Value;
}  // namespace base

namespace brave_wallet {

class KeyringService;
class SolanaInstruction;

class SolanaTransaction {
 public:
  struct SendOptions {
    SendOptions();
    ~SendOptions();
    SendOptions(const SendOptions&);
    SendOptions(absl::optional<uint64_t> max_retries_param,
                absl::optional<std::string> preflight_commitment_param,
                absl::optional<bool> skip_preflight_param);

    bool operator==(const SolanaTransaction::SendOptions&) const;
    bool operator!=(const SolanaTransaction::SendOptions&) const;

    static absl::optional<SendOptions> FromValue(
        const base::Value::Dict& value);
    static absl::optional<SendOptions> FromValue(
        absl::optional<base::Value::Dict> value);
    base::Value::Dict ToValue() const;
    static absl::optional<SendOptions> FromMojomSendOptions(
        mojom::SolanaSendTransactionOptionsPtr mojom_options);
    mojom::SolanaSendTransactionOptionsPtr ToMojomSendOptions() const;

    absl::optional<uint64_t> max_retries;
    absl::optional<std::string> preflight_commitment;
    absl::optional<bool> skip_preflight;
  };

  explicit SolanaTransaction(SolanaMessage&& message);
  SolanaTransaction(const std::string& recent_blockhash,
                    uint64_t last_valid_block_height,
                    const std::string& fee_payer,
                    std::vector<SolanaInstruction>&& instructions);
  SolanaTransaction(SolanaMessage&& message,
                    const std::vector<uint8_t>& raw_signatures);
  SolanaTransaction(SolanaMessage&& message,
                    mojom::SolanaSignTransactionParamPtr sign_tx_param);
  SolanaTransaction(const SolanaTransaction&) = delete;
  SolanaTransaction& operator=(const SolanaTransaction&) = delete;
  ~SolanaTransaction();
  bool operator==(const SolanaTransaction&) const;
  bool operator!=(const SolanaTransaction&) const;

  // Serialize the message and sign it.
  std::string GetSignedTransaction(KeyringService* keyring_service) const;
  // Serialize and encode the message in Base64.
  std::string GetBase64EncodedMessage() const;
  absl::optional<std::vector<uint8_t>> GetSignedTransactionBytes(
      KeyringService* keyring_service,
      const std::vector<uint8_t>* fee_payer_signature = nullptr) const;

  // Returns message bytes and signer addresses (public keys).
  absl::optional<std::pair<std::vector<uint8_t>, std::vector<std::string>>>
  GetSerializedMessage() const;

  mojom::SolanaTxDataPtr ToSolanaTxData() const;
  base::Value::Dict ToValue() const;

  static std::unique_ptr<SolanaTransaction> FromSolanaTxData(
      mojom::SolanaTxDataPtr solana_tx_data);
  static std::unique_ptr<SolanaTransaction> FromValue(
      const base::Value::Dict& value);
  static std::unique_ptr<SolanaTransaction> FromSignedTransactionBytes(
      const std::vector<uint8_t>& bytes);

  std::string to_wallet_address() const { return to_wallet_address_; }
  std::string spl_token_mint_address() const { return spl_token_mint_address_; }
  mojom::TransactionType tx_type() const { return tx_type_; }
  uint64_t lamports() const { return lamports_; }
  uint64_t amount() const { return amount_; }
  SolanaMessage* message() { return &message_; }
  const std::vector<uint8_t>& raw_signatures() const { return raw_signatures_; }
  absl::optional<SolanaTransaction::SendOptions> send_options() const {
    return send_options_;
  }

  void set_to_wallet_address(const std::string& to_wallet_address) {
    to_wallet_address_ = to_wallet_address;
  }
  void set_spl_token_mint_address(const std::string& spl_token_mint_address) {
    spl_token_mint_address_ = spl_token_mint_address;
  }
  void set_tx_type(mojom::TransactionType tx_type);
  void set_lamports(uint64_t lamports) { lamports_ = lamports; }
  void set_amount(uint64_t amount) { amount_ = amount; }
  void set_send_options(absl::optional<SendOptions> options) {
    send_options_ = options;
  }
  void set_sign_tx_param(mojom::SolanaSignTransactionParamPtr sign_tx_param) {
    sign_tx_param_ = std::move(sign_tx_param);
  }

 private:
  FRIEND_TEST_ALL_PREFIXES(SolanaTransactionUnitTest, GetBase64EncodedMessage);
  SolanaMessage message_;
  // Value will be assigned when FromSignedTransactionBytes is called.
  std::vector<uint8_t> raw_signatures_;

  // Passed by dApp when calling signAndSendTransaction, signTransaction,
  // signAllTransactions provider APIs, which includes serialized message and
  // signatures from partial_sign. If this exists, we will use the serialized
  // message inside when signing the transaction instead of serializing the
  // message by ourselves. This is the order of accounts with the same
  // is_signer and is_writable properties can be different across
  // implementations, we need to sign the exact serialized message being passed
  // by dApp.
  mojom::SolanaSignTransactionParamPtr sign_tx_param_ = nullptr;

  // Data fields to be used for UI, they are filled currently when we create
  // SolanaTxData to transfer SOL or SPL tokens for UI.
  std::string to_wallet_address_;
  std::string spl_token_mint_address_;
  mojom::TransactionType tx_type_ = mojom::TransactionType::Other;
  uint64_t lamports_ = 0;  // amount of lamports to transfer
  uint64_t amount_ = 0;    // amount of SPL tokens to transfer

  // Currently might be specified by solana.signAndSendTransaction provider
  // API as the options to be passed to sendTransaction RPC call.
  absl::optional<SendOptions> send_options_;
};

}  // namespace brave_wallet

#endif  // BRAVE_COMPONENTS_BRAVE_WALLET_BROWSER_SOLANA_TRANSACTION_H_
