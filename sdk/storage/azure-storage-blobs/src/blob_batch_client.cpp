// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/blobs/blob_batch_client.hpp"

#include <algorithm>
#include <cstring>
#include <memory>

#include "azure/core/http/policy.hpp"
#include "azure/storage/blobs/version.hpp"
#include "azure/storage/common/constants.hpp"
#include "azure/storage/common/shared_key_policy.hpp"
#include "azure/storage/common/storage_per_retry_policy.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  namespace {
    class NoopTransportPolicy : public Core::Http::HttpPolicy {
    public:
      ~NoopTransportPolicy() override {}

      std::unique_ptr<HttpPolicy> Clone() const override
      {
        return std::make_unique<NoopTransportPolicy>(*this);
      }

      std::unique_ptr<Core::Http::RawResponse> Send(
          Core::Context const& context,
          Core::Http::Request& request,
          Core::Http::NextHttpPolicy nextHttpPolicy) const override
      {
        unused(context, request, nextHttpPolicy);
        return std::unique_ptr<Core::Http::RawResponse>();
      }
    };
  } // namespace

  int32_t BlobBatch::DeleteBlob(
      const std::string& blobContainerName,
      const std::string& blobName,
      const DeleteBlobOptions& options)
  {
    DeleteBlobSubRequest operation;
    operation.BlobContainerName = blobContainerName;
    operation.BlobName = blobName;
    operation.Options = options;
    m_deleteBlobSubRequests.emplace_back(std::move(operation));
    return static_cast<int32_t>(m_deleteBlobSubRequests.size() - 1);
  }

  int32_t BlobBatch::SetBlobAccessTier(
      const std::string& blobContainerName,
      const std::string& blobName,
      Models::AccessTier tier,
      const SetBlobAccessTierOptions& options)
  {
    SetBlobAccessTierSubRequest operation;
    operation.BlobContainerName = blobContainerName;
    operation.BlobName = blobName;
    operation.Options = options;
    operation.Tier = tier;
    m_setBlobAccessTierSubRequests.emplace_back(std::move(operation));
    return static_cast<int32_t>(m_setBlobAccessTierSubRequests.size() - 1);
  }

  BlobBatchClient BlobBatchClient::CreateFromConnectionString(
      const std::string& connectionString,
      const BlobClientOptions& options)
  {
    auto parsedConnectionString = Storage::Details::ParseConnectionString(connectionString);
    auto serviceUrl = std::move(parsedConnectionString.BlobServiceUrl);

    if (parsedConnectionString.KeyCredential)
    {
      return BlobBatchClient(
          serviceUrl.GetAbsoluteUrl(), parsedConnectionString.KeyCredential, options);
    }
    else
    {
      return BlobBatchClient(serviceUrl.GetAbsoluteUrl(), options);
    }
  }

  BlobBatchClient::BlobBatchClient(
      const std::string& serviceUrl,
      std::shared_ptr<StorageSharedKeyCredential> credential,
      const BlobClientOptions& options)
      : m_serviceUrl(serviceUrl)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Storage::Details::BlobServicePackageName, Version::VersionString()));
    policies.emplace_back(std::make_unique<Azure::Core::Http::RequestIdPolicy>());
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(
        std::make_unique<Storage::Details::StorageRetryPolicy>(options.RetryOptions));
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<Storage::Details::StoragePerRetryPolicy>());
    policies.emplace_back(std::make_unique<Storage::Details::SharedKeyPolicy>(credential));
    policies.emplace_back(
        std::make_unique<Azure::Core::Http::TransportPolicy>(options.TransportPolicyOptions));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);

    policies.clear();
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<Storage::Details::StoragePerRetryPolicy>());
    policies.emplace_back(std::make_unique<Storage::Details::SharedKeyPolicy>(credential));
    policies.emplace_back(std::make_unique<NoopTransportPolicy>());
    m_subRequestPipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  BlobBatchClient::BlobBatchClient(
      const std::string& serviceUrl,
      std::shared_ptr<Core::TokenCredential> credential,
      const BlobClientOptions& options)
      : m_serviceUrl(serviceUrl)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Storage::Details::BlobServicePackageName, Version::VersionString()));
    policies.emplace_back(std::make_unique<Azure::Core::Http::RequestIdPolicy>());
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(
        std::make_unique<Storage::Details::StorageRetryPolicy>(options.RetryOptions));
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<Storage::Details::StoragePerRetryPolicy>());
    policies.emplace_back(std::make_unique<Core::Http::BearerTokenAuthenticationPolicy>(
        credential, Storage::Details::StorageScope));
    policies.emplace_back(
        std::make_unique<Azure::Core::Http::TransportPolicy>(options.TransportPolicyOptions));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);

    policies.clear();
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<Storage::Details::StoragePerRetryPolicy>());
    policies.emplace_back(std::make_unique<Core::Http::BearerTokenAuthenticationPolicy>(
        credential, Storage::Details::StorageScope));
    policies.emplace_back(std::make_unique<NoopTransportPolicy>());
    m_subRequestPipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  BlobBatchClient::BlobBatchClient(const std::string& serviceUrl, const BlobClientOptions& options)
      : m_serviceUrl(serviceUrl)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Storage::Details::BlobServicePackageName, Version::VersionString()));
    policies.emplace_back(std::make_unique<Azure::Core::Http::RequestIdPolicy>());
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(
        std::make_unique<Storage::Details::StorageRetryPolicy>(options.RetryOptions));
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<Storage::Details::StoragePerRetryPolicy>());
    policies.emplace_back(
        std::make_unique<Azure::Core::Http::TransportPolicy>(options.TransportPolicyOptions));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);

    policies.clear();
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<Storage::Details::StoragePerRetryPolicy>());
    policies.emplace_back(std::make_unique<NoopTransportPolicy>());
    m_subRequestPipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  Azure::Core::Response<SubmitBlobBatchResult> BlobBatchClient::SubmitBatch(
      const BlobBatch& batch,
      const SubmitBlobBatchOptions& options) const
  {
    const std::string LineEnding = "\r\n";
    const std::string ContentTypePrefix = "multipart/mixed; boundary=";

    std::string boundary = "batch_" + Azure::Core::Uuid::CreateUuid().GetUuidString();

    enum class RequestType
    {
      DeleteBlob,
      SetBlobAccessTier,
    };

    std::vector<RequestType> requestTypes;

    std::string requestBody;
    {
      auto getBatchBoundary = [&LineEnding, &boundary, subRequestCounter = 0]() mutable {
        std::string ret;
        ret += "--" + boundary + LineEnding;
        ret += "Content-Type: application/http" + LineEnding + "Content-Transfer-Encoding: binary"
            + LineEnding + "Content-ID: " + std::to_string(subRequestCounter++) + LineEnding
            + LineEnding;
        return ret;
      };
      for (const auto& subrequest : batch.m_deleteBlobSubRequests)
      {
        requestTypes.emplace_back(RequestType::DeleteBlob);

        requestBody += getBatchBoundary();

        auto blobUrl = m_serviceUrl;
        blobUrl.AppendPath(Storage::Details::UrlEncodePath(subrequest.BlobContainerName));
        blobUrl.AppendPath(Storage::Details::UrlEncodePath(subrequest.BlobName));
        Details::BlobRestClient::Blob::DeleteBlobOptions protocolLayerOptions;
        protocolLayerOptions.DeleteSnapshots = subrequest.Options.DeleteSnapshots;
        protocolLayerOptions.IfModifiedSince = subrequest.Options.AccessConditions.IfModifiedSince;
        protocolLayerOptions.IfUnmodifiedSince
            = subrequest.Options.AccessConditions.IfUnmodifiedSince;
        protocolLayerOptions.IfMatch = subrequest.Options.AccessConditions.IfMatch;
        protocolLayerOptions.IfNoneMatch = subrequest.Options.AccessConditions.IfNoneMatch;
        protocolLayerOptions.LeaseId = subrequest.Options.AccessConditions.LeaseId;
        auto message
            = Details::BlobRestClient::Blob::DeleteCreateMessage(blobUrl, protocolLayerOptions);
        message.RemoveHeader(Storage::Details::HttpHeaderXMsVersion);
        m_subRequestPipeline->Send(options.Context, message);
        requestBody += message.GetHTTPMessagePreBody();
      }
      for (const auto& subrequest : batch.m_setBlobAccessTierSubRequests)
      {
        requestTypes.emplace_back(RequestType::SetBlobAccessTier);

        requestBody += getBatchBoundary();

        auto blobUrl = m_serviceUrl;
        blobUrl.AppendPath(Storage::Details::UrlEncodePath(subrequest.BlobContainerName));
        blobUrl.AppendPath(Storage::Details::UrlEncodePath(subrequest.BlobName));
        Details::BlobRestClient::Blob::SetBlobAccessTierOptions protocolLayerOptions;
        protocolLayerOptions.Tier = subrequest.Tier;
        protocolLayerOptions.RehydratePriority = subrequest.Options.RehydratePriority;
        auto message = Details::BlobRestClient::Blob::SetAccessTierCreateMessage(
            blobUrl, protocolLayerOptions);
        message.RemoveHeader(Storage::Details::HttpHeaderXMsVersion);
        m_subRequestPipeline->Send(options.Context, message);
        requestBody += message.GetHTTPMessagePreBody();
      }
      requestBody += "--" + boundary + "--" + LineEnding;
    }

    Details::BlobRestClient::BlobBatch::SubmitBlobBatchOptions protocolLayerOptions;
    protocolLayerOptions.ContentType = ContentTypePrefix + boundary;

    Azure::Core::Http::MemoryBodyStream requestBodyStream(
        reinterpret_cast<const uint8_t*>(requestBody.data()), requestBody.length());

    auto rawResponse = Details::BlobRestClient::BlobBatch::SubmitBatch(
        options.Context, *m_pipeline, m_serviceUrl, &requestBodyStream, protocolLayerOptions);

    if (rawResponse->ContentType.substr(0, ContentTypePrefix.length()) == ContentTypePrefix)
    {
      boundary = rawResponse->ContentType.substr(ContentTypePrefix.length());
    }
    else
    {
      throw std::runtime_error("failed to parse Content-Type response header");
    }

    SubmitBlobBatchResult batchResult;
    {
      const std::vector<uint8_t>& responseBody = rawResponse.GetRawResponse().GetBody();

      const char* const startPos = reinterpret_cast<const char*>(responseBody.data());
      const char* currPos = startPos;
      const char* const endPos = currPos + responseBody.size();

      auto parseLookAhead = [&currPos, endPos](const std::string& expect) -> bool {
        // This doesn't move currPos
        for (std::size_t i = 0; i < expect.length(); ++i)
        {
          if (currPos + i < endPos && currPos[i] == expect[i])
          {
            continue;
          }
          return false;
        }
        return true;
      };

      auto parseConsume = [&currPos, startPos, &parseLookAhead](const std::string& expect) -> void {
        // This moves currPos
        if (parseLookAhead(expect))
        {
          currPos += expect.length();
        }
        else
        {
          throw std::runtime_error(
              "failed to parse response body at " + std::to_string(currPos - startPos));
        }
      };

      auto parseFindNext = [&currPos, endPos](const std::string& expect) -> const char* {
        // This doesn't move currPos
        return std::search(currPos, endPos, expect.begin(), expect.end());
      };

      auto parseFindNextAfter = [endPos, &parseFindNext](const std::string& expect) -> const char* {
        // This doesn't move currPos
        return std::min(endPos, parseFindNext(expect) + expect.length());
      };

      auto parseGetUntilAfter
          = [&currPos, endPos, &parseFindNext](const std::string& expect) -> std::string {
        // This moves currPos
        auto ePos = parseFindNext(expect);
        std::string ret(currPos, ePos);
        currPos = std::min(endPos, ePos + expect.length());
        return ret;
      };

      int subRequestCounter = 0;
      while (true)
      {
        parseConsume("--" + boundary);

        if (parseLookAhead("--"))
        {
          parseConsume("--");
        }

        if (currPos == endPos)
        {
          break;
        }

        currPos = parseFindNextAfter(LineEnding + LineEnding);
        auto boundaryPos = parseFindNext("--" + boundary);

        // now (currPos, boundaryPos) is a subresponse body
        parseConsume("HTTP/");
        int32_t httpMajorVersion = std::stoi(parseGetUntilAfter("."));
        int32_t httpMinorVersion = std::stoi(parseGetUntilAfter(" "));
        int32_t httpStatusCode = std::stoi(parseGetUntilAfter(" "));
        std::string httpReasonPhrase = parseGetUntilAfter(LineEnding);

        auto rawSubresponse = std::make_unique<Azure::Core::Http::RawResponse>(
            httpMajorVersion,
            httpMinorVersion,
            static_cast<Azure::Core::Http::HttpStatusCode>(httpStatusCode),
            httpReasonPhrase);

        while (currPos < boundaryPos)
        {
          if (parseLookAhead(LineEnding))
          {
            break;
          }

          std::string headerName = parseGetUntilAfter(": ");
          std::string headerValue = parseGetUntilAfter(LineEnding);
          rawSubresponse->AddHeader(headerName, headerValue);
        }

        parseConsume(LineEnding);

        rawSubresponse->SetBody(std::vector<uint8_t>(currPos, boundaryPos));
        currPos = boundaryPos;

        RequestType requestType = requestTypes[subRequestCounter++];
        if (requestType == RequestType::DeleteBlob)
        {
          try
          {
            batchResult.DeleteBlobResults.emplace_back(
                Details::BlobRestClient::Blob::DeleteCreateResponse(
                    options.Context, std::move(rawSubresponse)));
          }
          catch (StorageException& e)
          {
            batchResult.DeleteBlobResults.emplace_back(
                Azure::Core::Response<Models::DeleteBlobResult>(
                    Models::DeleteBlobResult{}, std::move(e.RawResponse)));
          }
        }
        else if (requestType == RequestType::SetBlobAccessTier)
        {
          try
          {
            batchResult.SetBlobAccessTierResults.emplace_back(
                Details::BlobRestClient::Blob::SetAccessTierCreateResponse(
                    options.Context, std::move(rawSubresponse)));
          }
          catch (StorageException& e)
          {
            batchResult.SetBlobAccessTierResults.emplace_back(
                Azure::Core::Response<Models::SetBlobAccessTierResult>(
                    Models::SetBlobAccessTierResult{}, std::move(e.RawResponse)));
          }
        }
      }
    }

    return Azure::Core::Response<SubmitBlobBatchResult>(
        std::move(batchResult), rawResponse.ExtractRawResponse());
  }
}}} // namespace Azure::Storage::Blobs
