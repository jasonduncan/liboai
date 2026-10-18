# Webhook verification

`Webhooks` manages remote endpoints; receiving events is an application HTTP-server responsibility. Verify the exact raw request bytes before parsing or dispatching:

```cpp
auto event = liboai::unwrap_webhook(raw_body, request_headers, signing_secret);
// Persist event["id"] before performing idempotent application work.
```

`verify_webhook_signature` validates without parsing; `unwrap_webhook` validates then returns unrestricted JSON, preserving unknown event types. Both accept optional timestamp tolerance (default 300 seconds) and a clock time for testing. Invalid/missing signatures, missing headers, and timestamps outside the tolerance throw `exception::WebhookVerificationError`. An invalid secret/tolerance throws `std::invalid_argument`; validly signed malformed JSON raises a JSON parse error.

The verifier uses HMAC-SHA256 from CommonCrypto on Apple platforms. cURL's TLS support does not provide a portable HMAC API. Other platforms can opt into the existing OpenSSL Crypto implementation with `-DLIBOAI_WEBHOOK_OPENSSL=ON`; this adds an explicit link dependency and also works on Apple. Without a provider, `webhook_verification_available()` returns false and verification throws `std::logic_error`. Endpoint management is still usable. No cryptographic hash or MAC primitive is implemented in liboai.

Secrets are base64, optionally prefixed `whsec_`. Required headers are `webhook-id`, `webhook-timestamp`, and `webhook-signature`, looked up case-insensitively. The signed message is the ID, timestamp text, and raw payload joined with periods. Whitespace-separated `v1,` signatures support key rotation. Both old and future timestamps outside the tolerance are rejected. Signature comparison examines all 32 digest bytes.

Pass the original byte string, including whitespace, UTF-8, and any embedded NULs; parsing and reserializing JSON changes the signed bytes. Your HTTP server should reject duplicate verification headers before converting them to `Headers`, since a map cannot represent multiple separately supplied values. Timestamp checks reduce replay windows but do not prevent repeat delivery: persist event IDs and process side effects idempotently. Keep signing secrets out of source control and logs.

Local tests use synthetic known HMAC vectors and tampering/timestamp cases. They do not establish delivery from OpenAI or endpoint account access. See the [official webhook guide](https://developers.openai.com/api/docs/guides/webhooks) and [endpoint examples](../resources/resources.md).
