// Copyright (c) 2013, Kenton Varda <temporal@gmail.com>
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "wire-format.h"
#include "descriptor.h"
#include "message.h"
#include <gtest/gtest.h>

namespace capnproto {
  template <typename T, typename U>
  std::ostream& operator<<(std::ostream& os, Quantity<T, U> value) {
    return os << (value / unit<Quantity<T, U>>());
  }
}

namespace capnproto {
namespace internal {
namespace {

TEST(WireFormat, SimpleRawDataStruct) {
  AlignedData<2> data = {{
    // Struct ref, offset = 1, fieldCount = 1, dataSize = 1, referenceCount = 0
    0x08, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00,
    // Content for the data segment.
    0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef
  }};

  StructReader reader = StructReader::readRootTrusted(data.words, data.words);

  EXPECT_EQ(0xefcdab8967452301ull, reader.getDataField<uint64_t>(0 * ELEMENTS, 321u));
  EXPECT_EQ(321u, reader.getDataField<uint64_t>(1 * ELEMENTS, 321u));
  EXPECT_EQ(0x67452301u, reader.getDataField<uint32_t>(0 * ELEMENTS, 321u));
  EXPECT_EQ(0xefcdab89u, reader.getDataField<uint32_t>(1 * ELEMENTS, 321u));
  EXPECT_EQ(321u, reader.getDataField<uint32_t>(2 * ELEMENTS, 321u));
  EXPECT_EQ(0x2301u, reader.getDataField<uint16_t>(0 * ELEMENTS, 321u));
  EXPECT_EQ(0x6745u, reader.getDataField<uint16_t>(1 * ELEMENTS, 321u));
  EXPECT_EQ(0xab89u, reader.getDataField<uint16_t>(2 * ELEMENTS, 321u));
  EXPECT_EQ(0xefcdu, reader.getDataField<uint16_t>(3 * ELEMENTS, 321u));
  EXPECT_EQ(321u, reader.getDataField<uint16_t>(4 * ELEMENTS, 321u));

  // Bits
  EXPECT_TRUE (reader.getDataField<bool>(0 * ELEMENTS, false));
  EXPECT_FALSE(reader.getDataField<bool>(1 * ELEMENTS, false));
  EXPECT_FALSE(reader.getDataField<bool>(2 * ELEMENTS, false));
  EXPECT_FALSE(reader.getDataField<bool>(3 * ELEMENTS, false));
  EXPECT_FALSE(reader.getDataField<bool>(4 * ELEMENTS, false));
  EXPECT_FALSE(reader.getDataField<bool>(5 * ELEMENTS, false));
  EXPECT_FALSE(reader.getDataField<bool>(6 * ELEMENTS, false));
  EXPECT_FALSE(reader.getDataField<bool>(7 * ELEMENTS, false));

  EXPECT_TRUE (reader.getDataField<bool>( 8 * ELEMENTS, false));
  EXPECT_TRUE (reader.getDataField<bool>( 9 * ELEMENTS, false));
  EXPECT_FALSE(reader.getDataField<bool>(10 * ELEMENTS, false));
  EXPECT_FALSE(reader.getDataField<bool>(11 * ELEMENTS, false));
  EXPECT_FALSE(reader.getDataField<bool>(12 * ELEMENTS, false));
  EXPECT_TRUE (reader.getDataField<bool>(13 * ELEMENTS, false));
  EXPECT_FALSE(reader.getDataField<bool>(14 * ELEMENTS, false));
  EXPECT_FALSE(reader.getDataField<bool>(15 * ELEMENTS, false));

  EXPECT_TRUE (reader.getDataField<bool>(63 * ELEMENTS, false));
  EXPECT_TRUE (reader.getDataField<bool>(63 * ELEMENTS, true ));
  EXPECT_FALSE(reader.getDataField<bool>(64 * ELEMENTS, false));
  EXPECT_TRUE (reader.getDataField<bool>(64 * ELEMENTS, true ));

  // Field number guards.
  EXPECT_EQ(0xefcdab89u,
      reader.getDataFieldCheckingNumber<uint32_t>(FieldNumber(0), 1 * ELEMENTS, 321u));
  EXPECT_EQ(321u,
      reader.getDataFieldCheckingNumber<uint32_t>(FieldNumber(1), 1 * ELEMENTS, 321u));

  EXPECT_TRUE (reader.getDataFieldCheckingNumber<bool>(FieldNumber(0), 0 * ELEMENTS, false));
  EXPECT_TRUE (reader.getDataFieldCheckingNumber<bool>(FieldNumber(0), 0 * ELEMENTS, true ));
  EXPECT_FALSE(reader.getDataFieldCheckingNumber<bool>(FieldNumber(0), 1 * ELEMENTS, false));
  EXPECT_FALSE(reader.getDataFieldCheckingNumber<bool>(FieldNumber(0), 1 * ELEMENTS, true ));
  EXPECT_FALSE(reader.getDataFieldCheckingNumber<bool>(FieldNumber(1), 0 * ELEMENTS, false));
  EXPECT_TRUE (reader.getDataFieldCheckingNumber<bool>(FieldNumber(1), 0 * ELEMENTS, true ));
}

static void setupStruct(StructBuilder builder) {
  builder.setDataField<uint64_t>(0 * ELEMENTS, 0x1011121314151617ull);
  builder.setDataField<uint32_t>(2 * ELEMENTS, 0x20212223u);
  builder.setDataField<uint16_t>(6 * ELEMENTS, 0x3031u);
  builder.setDataField<uint8_t>(14 * ELEMENTS, 0x40u);
  builder.setDataField<bool>(120 * ELEMENTS, false);
  builder.setDataField<bool>(121 * ELEMENTS, false);
  builder.setDataField<bool>(122 * ELEMENTS, true);
  builder.setDataField<bool>(123 * ELEMENTS, false);
  builder.setDataField<bool>(124 * ELEMENTS, true);
  builder.setDataField<bool>(125 * ELEMENTS, true);
  builder.setDataField<bool>(126 * ELEMENTS, true);
  builder.setDataField<bool>(127 * ELEMENTS, false);

  {
    StructBuilder subStruct = builder.getStructField(
        0 * REFERENCES, FieldNumber(1), 1 * WORDS, 0 * REFERENCES);
    subStruct.setDataField<uint32_t>(0 * ELEMENTS, 123);
  }

  {
    ListBuilder list = builder.initListField(1 * REFERENCES, FieldSize::FOUR_BYTES, 3 * ELEMENTS);
    EXPECT_EQ(3 * ELEMENTS, list.size());
    list.setDataElement<int32_t>(0 * ELEMENTS, 200);
    list.setDataElement<int32_t>(1 * ELEMENTS, 201);
    list.setDataElement<int32_t>(2 * ELEMENTS, 202);
  }

  {
    ListBuilder list = builder.initStructListField(
        2 * REFERENCES, 4 * ELEMENTS, FieldNumber(2), 1 * WORDS, 1 * REFERENCES);
    EXPECT_EQ(4 * ELEMENTS, list.size());
    for (int i = 0; i < 4; i++) {
      StructBuilder element = list.getStructElement(i * ELEMENTS, 2 * WORDS / ELEMENTS, 1 * WORDS);
      element.setDataField<int32_t>(0 * ELEMENTS, 300 + i);
      element.getStructField(0 * REFERENCES, FieldNumber(1), 1 * WORDS, 0 * REFERENCES)
             .setDataField<int32_t>(0 * ELEMENTS, 400 + i);
    }
  }

  {
    ListBuilder list = builder.initListField(3 * REFERENCES, FieldSize::REFERENCE, 5 * ELEMENTS);
    EXPECT_EQ(5 * ELEMENTS, list.size());
    for (uint i = 0; i < 5; i++) {
      ListBuilder element = list.initListElement(
          i * REFERENCES, FieldSize::TWO_BYTES, (i + 1) * ELEMENTS);
      EXPECT_EQ((i + 1) * ELEMENTS, element.size());
      for (uint j = 0; j <= i; j++) {
        element.setDataElement<uint16_t>(j * ELEMENTS, 500 + j);
      }
    }
  }
}

static void checkStruct(StructReader reader) {
  EXPECT_EQ(0x1011121314151617ull, reader.getDataField<uint64_t>(0 * ELEMENTS, 1616));
  EXPECT_EQ(0x20212223u, reader.getDataField<uint32_t>(2 * ELEMENTS, 1616));
  EXPECT_EQ(0x3031u, reader.getDataField<uint16_t>(6 * ELEMENTS, 1616));
  EXPECT_EQ(0x40u, reader.getDataField<uint8_t>(14 * ELEMENTS, 16));
  EXPECT_FALSE(reader.getDataField<bool>(120 * ELEMENTS, false));
  EXPECT_FALSE(reader.getDataField<bool>(121 * ELEMENTS, false));
  EXPECT_TRUE (reader.getDataField<bool>(122 * ELEMENTS, false));
  EXPECT_FALSE(reader.getDataField<bool>(123 * ELEMENTS, false));
  EXPECT_TRUE (reader.getDataField<bool>(124 * ELEMENTS, false));
  EXPECT_TRUE (reader.getDataField<bool>(125 * ELEMENTS, false));
  EXPECT_TRUE (reader.getDataField<bool>(126 * ELEMENTS, false));
  EXPECT_FALSE(reader.getDataField<bool>(127 * ELEMENTS, false));

  {
    // TODO:  Use valid default value.
    StructReader subStruct = reader.getStructField(0 * REFERENCES, nullptr);
    EXPECT_EQ(123u, subStruct.getDataField<uint32_t>(0 * ELEMENTS, 456));
  }

  {
    // TODO:  Use valid default value.
    ListReader list = reader.getListField(1 * REFERENCES, FieldSize::FOUR_BYTES, nullptr);
    ASSERT_EQ(3 * ELEMENTS, list.size());
    EXPECT_EQ(200, list.getDataElement<int32_t>(0 * ELEMENTS));
    EXPECT_EQ(201, list.getDataElement<int32_t>(1 * ELEMENTS));
    EXPECT_EQ(202, list.getDataElement<int32_t>(2 * ELEMENTS));
  }

  {
    // TODO:  Use valid default value.
    ListReader list = reader.getListField(2 * REFERENCES, FieldSize::STRUCT, nullptr);
    ASSERT_EQ(4 * ELEMENTS, list.size());
    for (int i = 0; i < 4; i++) {
      StructReader element = list.getStructElement(i * ELEMENTS, nullptr);
      EXPECT_EQ(300 + i, element.getDataField<int32_t>(0 * ELEMENTS, 1616));
      EXPECT_EQ(400 + i,
          element.getStructField(0 * REFERENCES, nullptr)
              .getDataField<int32_t>(0 * ELEMENTS, 1616));
    }
  }

  {
    // TODO:  Use valid default value.
    ListReader list = reader.getListField(3 * REFERENCES, FieldSize::REFERENCE, nullptr);
    ASSERT_EQ(5 * ELEMENTS, list.size());
    for (uint i = 0; i < 5; i++) {
      ListReader element = list.getListElement(i * REFERENCES, FieldSize::TWO_BYTES, nullptr);
      ASSERT_EQ((i + 1) * ELEMENTS, element.size());
      for (uint j = 0; j <= i; j++) {
        EXPECT_EQ(500u + j, element.getDataElement<uint16_t>(j * ELEMENTS));
      }
    }
  }
}

TEST(WireFormat, StructRoundTrip) {
  std::unique_ptr<MessageBuilder> message = newMallocMessage(512 * WORDS);
  SegmentBuilder* segment = message->getSegmentWithAvailable(1 * WORDS);
  word* rootLocation = segment->allocate(1 * WORDS);

  StructBuilder builder =
      StructBuilder::initRoot(segment, rootLocation, FieldNumber(16), 2 * WORDS, 4 * REFERENCES);
  setupStruct(builder);

  // word count:
  //    1  root reference
  //    6  root struct
  //    1  sub message
  //    2  3-element int32 list
  //   13  struct list
  //         1 tag
  //        12 4x struct
  //           1 data segment
  //           1 reference segment
  //           1 sub-struct
  //   11  list list
  //         5 references to sub-lists
  //         6 sub-lists (4x 1 word, 1x 2 words)
  // -----
  //   34
  EXPECT_EQ(34 * WORDS, segment->getSize());

  checkStruct(builder.asReader());
  checkStruct(StructReader::readRootTrusted(segment->getStartPtr(), nullptr));
  checkStruct(StructReader::readRoot(segment->getStartPtr(), nullptr, segment, 4));
}

TEST(WireFormat, StructRoundTrip_MultipleSegments) {
  std::unique_ptr<MessageBuilder> message = newMallocMessage(1 * WORDS);
  SegmentBuilder* segment = message->getSegmentWithAvailable(1 * WORDS);
  word* rootLocation = segment->allocate(1 * WORDS);

  StructBuilder builder =
      StructBuilder::initRoot(segment, rootLocation, FieldNumber(16), 2 * WORDS, 4 * REFERENCES);
  setupStruct(builder);

  // Verify that we made 15 segments.
  ASSERT_TRUE(message->tryGetSegment(SegmentId(14)) != nullptr);
  EXPECT_EQ(nullptr, message->tryGetSegment(SegmentId(15)));

  // Check that each segment has the expected size.  Recall that the first word of each segment will
  // actually be a reference to the first thing allocated within that segment.
  EXPECT_EQ( 1 * WORDS, message->getSegment(SegmentId( 0))->getSize());  // root ref
  EXPECT_EQ( 7 * WORDS, message->getSegment(SegmentId( 1))->getSize());  // root struct
  EXPECT_EQ( 2 * WORDS, message->getSegment(SegmentId( 2))->getSize());  // sub-struct
  EXPECT_EQ( 3 * WORDS, message->getSegment(SegmentId( 3))->getSize());  // 3-element int32 list
  EXPECT_EQ(10 * WORDS, message->getSegment(SegmentId( 4))->getSize());  // struct list
  EXPECT_EQ( 2 * WORDS, message->getSegment(SegmentId( 5))->getSize());  // struct list substruct 1
  EXPECT_EQ( 2 * WORDS, message->getSegment(SegmentId( 6))->getSize());  // struct list substruct 2
  EXPECT_EQ( 2 * WORDS, message->getSegment(SegmentId( 7))->getSize());  // struct list substruct 3
  EXPECT_EQ( 2 * WORDS, message->getSegment(SegmentId( 8))->getSize());  // struct list substruct 4
  EXPECT_EQ( 6 * WORDS, message->getSegment(SegmentId( 9))->getSize());  // list list
  EXPECT_EQ( 2 * WORDS, message->getSegment(SegmentId(10))->getSize());  // list list sublist 1
  EXPECT_EQ( 2 * WORDS, message->getSegment(SegmentId(11))->getSize());  // list list sublist 2
  EXPECT_EQ( 2 * WORDS, message->getSegment(SegmentId(12))->getSize());  // list list sublist 3
  EXPECT_EQ( 2 * WORDS, message->getSegment(SegmentId(13))->getSize());  // list list sublist 4
  EXPECT_EQ( 3 * WORDS, message->getSegment(SegmentId(14))->getSize());  // list list sublist 5

  checkStruct(builder.asReader());
}

}  // namespace
}  // namespace internal
}  // namespace capnproto
