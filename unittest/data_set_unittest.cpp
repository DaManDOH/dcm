#include "gtest/gtest.h"

#include "dcm/data_set.h"
#include "dcm/dicom_reader.h"
#include "dcm/full_read_handler.h"

extern std::string g_data_dir;

TEST(DataSetTest, Append) {
  dcm::DataSet data_set;

  EXPECT_EQ(0, data_set.size());

  data_set.Append(new dcm::DataElement(0x00080005));
  data_set.Append(new dcm::DataElement(0x00080008));
  data_set.Append(new dcm::DataElement(0x00100010));

  EXPECT_EQ(3, data_set.size());

  EXPECT_EQ(0x00080005, data_set[0]->tag());
  EXPECT_EQ(0x00080008, data_set[1]->tag());
  EXPECT_EQ(0x00100010, data_set[2]->tag());
}

TEST(DataSetTest, InsertOrdered) {
  dcm::DataSet data_set;

  data_set.Insert(new dcm::DataElement(0x00080005));
  data_set.Insert(new dcm::DataElement(0x00080008));
  data_set.Insert(new dcm::DataElement(0x00100010));

  EXPECT_EQ(3, data_set.size());

  EXPECT_EQ(0x00080005, data_set[0]->tag());
  EXPECT_EQ(0x00080008, data_set[1]->tag());
  EXPECT_EQ(0x00100010, data_set[2]->tag());
}

TEST(DataSetTest, InsertUnordered) {
  dcm::DataSet data_set;

  data_set.Insert(new dcm::DataElement(0x00080008));
  data_set.Insert(new dcm::DataElement(0x00100010));
  data_set.Insert(new dcm::DataElement(0x00080005));

  EXPECT_EQ(3, data_set.size());

  EXPECT_EQ(0x00080005, data_set[0]->tag());
  EXPECT_EQ(0x00080008, data_set[1]->tag());
  EXPECT_EQ(0x00100010, data_set[2]->tag());
}

TEST(DataSetTest, InsertDuplicated) {
  dcm::DataSet data_set;
  bool ok = false;

  ok = data_set.Insert(new dcm::DataElement(0x00080008));
  EXPECT_TRUE(ok);

  ok = data_set.Insert(new dcm::DataElement(0x00080008));
  EXPECT_FALSE(ok);

  ok = data_set.Insert(new dcm::DataElement(0x00080008));
  EXPECT_FALSE(ok);

  EXPECT_EQ(1, data_set.size());

  EXPECT_EQ(0x00080008, data_set[0]->tag());
}

TEST(DataSetTest, GetElement) {
  dcm::DataSet data_set;

  EXPECT_TRUE(data_set.Get(0x00080008) == nullptr);

  data_set.Append(new dcm::DataElement(0x00080005));
  data_set.Append(new dcm::DataElement(0x00080008));
  data_set.Append(new dcm::DataElement(0x00100010));

  EXPECT_TRUE(data_set.Get(0x00080008) != nullptr);
  EXPECT_TRUE(data_set.Get(0x00080005) != nullptr);
  EXPECT_TRUE(data_set.Get(0x00100010) != nullptr);
  EXPECT_TRUE(data_set.Get(0x00020001) == nullptr);
}

TEST(DataSetTest, SetString) {
  dcm::Path path(g_data_dir);
  path /= "cs";
  path /= "Explicit Little (Ceph).dcm";

  dcm::DataSet data_set;
  dcm::FullReadHandler read_handler(&data_set);
  dcm::DicomReader reader(&read_handler);

  bool ok = reader.ReadFile(path);
  EXPECT_TRUE(ok);

  {
    std::string comments(10, 'a');

    ok = data_set.SetString(dcm::tags::kImageComments, comments);
    EXPECT_TRUE(ok);

    EXPECT_EQ(10, data_set.Get(dcm::tags::kImageComments)->length());

    std::string value;
    EXPECT_TRUE(data_set.GetString(dcm::tags::kImageComments, &value));
    EXPECT_EQ(comments, value);
  }

  {
    std::string comments(11, 'a');

    ok = data_set.SetString(dcm::tags::kImageComments, comments);
    EXPECT_TRUE(ok);

    EXPECT_EQ(12, data_set.Get(dcm::tags::kImageComments)->length());

    std::string value;
    EXPECT_TRUE(data_set.GetString(dcm::tags::kImageComments, &value));
    EXPECT_EQ(comments, value);
  }

  {
    ok = data_set.SetString(dcm::tags::kImageComments, "");
    EXPECT_TRUE(ok);

    EXPECT_EQ(0, data_set.Get(dcm::tags::kImageComments)->length());

    std::string value;
    EXPECT_TRUE(data_set.GetString(dcm::tags::kImageComments, &value));
    EXPECT_EQ("", value);
  }
}

TEST(DataSetTest, SetString_NoExist) {
  const std::string kAETitle = "MYSTORESCU";

  dcm::DataSet data_set;

  bool ok = data_set.SetString(dcm::tags::kSourceAETitle, kAETitle);
  EXPECT_TRUE(ok);

  EXPECT_EQ(1, data_set.size());

  std::string ae_title;
  data_set.GetString(dcm::tags::kSourceAETitle, &ae_title);
  EXPECT_EQ(kAETitle, ae_title);
}

TEST(DataSetTest, SetString_InvalidValue) {
  const std::string kAETitle = "MYSTORESCU_MORETHAN_MAX_LENGTH_16";

  dcm::DataSet data_set;

  bool ok = data_set.SetString(dcm::tags::kSourceAETitle, kAETitle);
  EXPECT_FALSE(ok);

  EXPECT_EQ(0, data_set.size());
}

TEST(DataSetTest, SetStringArray) {
  dcm::DataSet data_set;

  std::vector<std::string> values = {
    "0.127000", "0.127000"
  };

  bool ok = data_set.SetStringArray(dcm::tags::kPixelSpacing, values);
  EXPECT_TRUE(ok);

  std::string value;
  data_set.GetString(dcm::tags::kPixelSpacing, &value);

  EXPECT_EQ("0.127000\\0.127000", value);
}

#if 0
TEST(DataSetTest, GetGroupLength) {
  dcm::Path path(g_data_dir);
  path /= "cs";
  path /= "Explicit Little (Ceph).dcm";

  dcm::DataSet data_set;
  dcm::FullReadHandler read_handler(&data_set);
  dcm::DicomReader reader(&read_handler);

  bool ok = reader.ReadFile(path);
  EXPECT_TRUE(ok);

  EXPECT_EQ(178, data_set.GetGroupLength(2));
}
#endif  // 0

TEST(DataSetTest, UpdateGroupLength) {
  dcm::Path path(g_data_dir);
  path /= "cs";
  path /= "Explicit Little (Ceph).dcm";

  dcm::DataSet data_set;
  dcm::FullReadHandler read_handler(&data_set);
  dcm::DicomReader reader(&read_handler);

  bool ok = reader.ReadFile(path);
  EXPECT_TRUE(ok);

  {
    std::uint32_t group_length = 0;
    ok = data_set.GetUint32(0x00020000, &group_length);

    EXPECT_TRUE(ok);

    EXPECT_EQ(178, group_length);

    ok = data_set.UpdateGroupLength(2);
    EXPECT_TRUE(ok);

    group_length = 0;
    data_set.GetUint32(0x00020000, &group_length);
    EXPECT_EQ(178, group_length);
  }
}
