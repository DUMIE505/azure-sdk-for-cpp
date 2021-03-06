// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/storage/blobs/blob_responses.hpp"
#include "azure/storage/files/datalake/protocol/datalake_rest_client.hpp"

namespace Azure { namespace Storage { namespace Files { namespace DataLake { namespace Models {

  // ServiceClient models:

  using GetUserDelegationKeyResult = Blobs::Models::GetUserDelegationKeyResult;
  using UserDelegationKey = Blobs::Models::UserDelegationKey;
  using ListFileSystemsSegmentResult = ServiceListFileSystemsResult;

  // FileSystemClient models:

  using DeleteFileSystemResult = FileSystemDeleteResult;
  using ListPathsResult = FileSystemListPathsResult;

  struct GetFileSystemPropertiesResult
  {
    std::string ETag;
    std::string LastModified;
    Storage::Metadata Metadata;
  };

  using CreateFileSystemResult = FileSystemCreateResult;
  using SetFileSystemMetadataResult = FileSystemCreateResult;

  // PathClient models:

  using DeletePathResult = PathDeleteResult;
  using AcquirePathLeaseResult = Blobs::Models::AcquireBlobLeaseResult;
  using RenewPathLeaseResult = Blobs::Models::RenewBlobLeaseResult;
  using ReleasePathLeaseResult = Blobs::Models::ReleaseBlobLeaseResult;
  using ChangePathLeaseResult = Blobs::Models::ChangeBlobLeaseResult;
  using BreakPathLeaseResult = Blobs::Models::BreakBlobLeaseResult;

  struct Acl
  {
    std::string Scope;
    std::string Type;
    std::string Id;
    std::string Permissions;

    /**
     * @brief Creates an Acl based on acl input string.
     * @param aclString the string to be parsed to Acl.
     * @return Acl
     */
    static Acl FromString(const std::string& aclString);

    /**
     * @brief Creates a string from an Acl.
     * @param acl the acl object to be serialized to a string.
     * @return std::string
     */
    static std::string ToString(const Acl& acl);

    /**
     * @brief Creates a vector of Acl from a string that indicates multiple acls.
     * @param dataLakeAclsString the string that contains multiple acls.
     * @return std::vector<Acl>
     */
    static std::vector<Acl> DeserializeAcls(const std::string& dataLakeAclsString);

    /**
     * @brief Creates a string that contains several Acls.
     * @param dataLakeAclsArray the acls to be serialized into a string.
     * @return std::string
     */
    static std::string SerializeAcls(const std::vector<Acl>& dataLakeAclsArray);
  };

  struct GetPathPropertiesResult
  {
    std::string ETag;
    std::string LastModified;
    std::string CreationTime;
    int64_t ContentLength;
    Storage::Metadata Metadata;
    Azure::Core::Nullable<std::string> LeaseDuration;
    Azure::Core::Nullable<LeaseStateType> LeaseState;
    Azure::Core::Nullable<LeaseStatusType> LeaseStatus;
    DataLakeHttpHeaders HttpHeaders;
    Azure::Core::Nullable<bool> ServerEncrypted;
    Azure::Core::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
    Azure::Core::Nullable<bool> AccessTierInferred;
    Azure::Core::Nullable<std::string> AccessTierChangeTime;
    Azure::Core::Nullable<std::string> CopyId;
    Azure::Core::Nullable<std::string> CopySource;
    Azure::Core::Nullable<Blobs::Models::CopyStatus> CopyStatus;
    Azure::Core::Nullable<std::string> CopyProgress;
    Azure::Core::Nullable<std::string> CopyCompletionTime;
    Azure::Core::Nullable<std::string> ExpiryTime;
    Azure::Core::Nullable<std::string> LastAccessTime;
  };

  struct GetPathAccessControlResult
  {
    std::string ETag;
    std::string LastModified;
    std::vector<Acl> Acls;
  };

  struct SetPathHttpHeadersResult
  {
    std::string ETag;
    std::string LastModified;
  };

  struct SetPathMetadataResult
  {
    std::string ETag;
    std::string LastModified;
  };

  struct CreatePathResult
  {
    std::string ETag;
    std::string LastModified;
    Azure::Core::Nullable<int64_t> ContentLength;
  };

  using SetPathAccessControlResult = PathSetAccessControlResult;

  // FileClient models:

  using UploadFileFromResult = Blobs::Models::UploadBlockBlobResult;
  using AppendFileDataResult = PathAppendDataResult;
  using FlushFileDataResult = PathFlushDataResult;
  using ScheduleFileDeletionResult = Blobs::Models::SetBlobExpiryResult;

  struct ReadFileResult
  {
    std::unique_ptr<Azure::Core::Http::BodyStream> Body;
    DataLakeHttpHeaders HttpHeaders;
    Azure::Core::Nullable<int64_t> RangeOffset;
    Azure::Core::Nullable<int64_t> RangeLength;
    Azure::Core::Nullable<std::string> TransactionalMd5;
    std::string ETag;
    std::string LastModified;
    Azure::Core::Nullable<std::string> LeaseDuration;
    LeaseStateType LeaseState = LeaseStateType::Unknown;
    LeaseStatusType LeaseStatus = LeaseStatusType::Unknown;
    Azure::Core::Nullable<std::string> ContentMd5;
    Storage::Metadata Metadata;
    std::string CreationTime;
    Azure::Core::Nullable<std::string> ExpiryTime;
    Azure::Core::Nullable<std::string> LastAccessTime;
  };

  struct RenameFileResult
  {
  };

  struct DeleteFileResult
  {
  };

  struct DownloadFileToResult
  {
    std::string ETag;
    std::string LastModified;
    int64_t ContentLength = 0;
    DataLakeHttpHeaders HttpHeaders;
    Storage::Metadata Metadata;
    Azure::Core::Nullable<bool> ServerEncrypted;
    Azure::Core::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
  };

  using CreateFileResult = CreatePathResult;

  // DirectoryClient models:

  struct RenameDirectoryResult
  {
    Azure::Core::Nullable<std::string> ContinuationToken;
  };

  using SetDirectoryAccessControlRecursiveResult = PathSetAccessControlRecursiveResult;
  using CreateDirectoryResult = CreatePathResult;
  using DeleteDirectoryResult = PathDeleteResult;

}}}}} // namespace Azure::Storage::Files::DataLake::Models
