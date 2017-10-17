/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MPEG4_EXTRACTOR_H_

#define MPEG4_EXTRACTOR_H_

#include <arpa/inet.h>

#include <media/DataSource.h>
#include <media/MediaExtractor.h>
#include <utils/List.h>
#include <utils/Vector.h>
#include <utils/String8.h>

namespace android {
struct AMessage;
class DataSource;
class SampleTable;
class String8;
namespace heif {
class ItemTable;
}
using heif::ItemTable;

struct SidxEntry {
    size_t mSize;
    uint32_t mDurationUs;
};

struct Trex {
    uint32_t track_ID;
    uint32_t default_sample_description_index;
    uint32_t default_sample_duration;
    uint32_t default_sample_size;
    uint32_t default_sample_flags;
};

class MPEG4Extractor : public MediaExtractor {
public:
    // Extractor assumes ownership of "source".
    explicit MPEG4Extractor(const sp<DataSource> &source);

    virtual size_t countTracks();
    virtual sp<MediaSource> getTrack(size_t index);
    virtual sp<MetaData> getTrackMetaData(size_t index, uint32_t flags);

    virtual sp<MetaData> getMetaData();
    virtual uint32_t flags() const;
    virtual const char * name() { return "MPEG4Extractor"; }
    virtual void release();

    // for DRM
    virtual char* getDrmTrackInfo(size_t trackID, int *len);

protected:
    virtual ~MPEG4Extractor();

    virtual void populateMetrics();

private:

    struct PsshInfo {
        uint8_t uuid[16];
        uint32_t datalen;
        uint8_t *data;
    };
    struct Track {
        Track *next;
        sp<MetaData> meta;
        uint32_t timescale;
        sp<SampleTable> sampleTable;
        bool includes_expensive_metadata;
        bool skipTrack;
        bool has_elst;
        int64_t elst_media_time;
        uint64_t elst_segment_duration;
    };

    Vector<SidxEntry> mSidxEntries;
    off64_t mMoofOffset;
    bool mMoofFound;
    bool mMdatFound;

    Vector<PsshInfo> mPssh;

    Vector<Trex> mTrex;

    sp<DataSource> mDataSource;
    status_t mInitCheck;
    uint32_t mHeaderTimescale;
    bool mIsQT;
    bool mIsHEIF;

    Track *mFirstTrack, *mLastTrack;

    sp<MetaData> mFileMetaData;

    Vector<uint32_t> mPath;
    String8 mLastCommentMean;
    String8 mLastCommentName;
    String8 mLastCommentData;

    KeyedVector<uint32_t, AString> mMetaKeyMap;

    status_t readMetaData();
    status_t parseChunk(off64_t *offset, int depth);
    status_t parseITunesMetaData(off64_t offset, size_t size);
    status_t parseColorInfo(off64_t offset, size_t size);
    status_t parse3GPPMetaData(off64_t offset, size_t size, int depth);
    void parseID3v2MetaData(off64_t offset);
    status_t parseQTMetaKey(off64_t data_offset, size_t data_size);
    status_t parseQTMetaVal(int32_t keyId, off64_t data_offset, size_t data_size);

    status_t updateAudioTrackInfoFromESDS_MPEG4Audio(
            const void *esds_data, size_t esds_size);

    static status_t verifyTrack(Track *track);

    struct SINF {
        SINF *next;
        uint16_t trackID;
        uint8_t IPMPDescriptorID;
        ssize_t len;
        char *IPMPData;
    };

    SINF *mFirstSINF;

    bool mIsDrm;
    sp<ItemTable> mItemTable;

    status_t parseDrmSINF(off64_t *offset, off64_t data_offset);

    status_t parseTrackHeader(off64_t data_offset, off64_t data_size);

    status_t parseSegmentIndex(off64_t data_offset, size_t data_size);

    Track *findTrackByMimePrefix(const char *mimePrefix);

    status_t parseAC3SampleEntry(off64_t offset);
    status_t parseAC3SpecificBox(off64_t offset, uint16_t sampleRate);

    MPEG4Extractor(const MPEG4Extractor &);
    MPEG4Extractor &operator=(const MPEG4Extractor &);
};

bool SniffMPEG4(
        const sp<DataSource> &source, String8 *mimeType, float *confidence,
        sp<AMessage> *);

}  // namespace android

#endif  // MPEG4_EXTRACTOR_H_
