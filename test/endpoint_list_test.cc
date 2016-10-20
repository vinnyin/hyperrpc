#include <gtestx/gtestx.h>
#include "hyperrpc/endpoint_list.h"

class EndpointListTest : public testing::Test
{
protected:
  virtual void SetUp() {
    builder_ = new hrpc::EndpointListBuilderImpl(&endpoint_list_);
  }

  virtual void TearDown() {
    delete static_cast<hrpc::EndpointListBuilderImpl*>(builder_);
  }

  hrpc::EndpointList endpoint_list_;
  hrpc::EndpointListBuilder* builder_;
};

TEST_F(EndpointListTest, Builder)
{
  ASSERT_EQ(0, builder_->Size());
  builder_->PushBack({"127.0.0.1", 1234});
  ASSERT_EQ(1, builder_->Size());
  builder_->PushBack({"127.0.0.1", 5678});
  ASSERT_EQ(2, builder_->Size());
  builder_->Clear();
  ASSERT_EQ(0, builder_->Size());
  builder_->PushBack({"127.0.0.1", 1234});
  ASSERT_EQ(1, builder_->Size());
}

PERF_TEST_F(EndpointListTest, BuilderPerf)
{
  static hrpc::Addr addr{"127.0.0.1", 1234};
  builder_->PushBack(addr);
  builder_->Clear();
}

TEST_F(EndpointListTest, Access)
{
  ASSERT_TRUE(endpoint_list_.empty());
  endpoint_list_.PushBack({"127.0.0.1", 1000});
  ASSERT_TRUE(!endpoint_list_.empty());
  endpoint_list_.PushBack({"127.0.0.1", 1001});
  ASSERT_EQ(2, endpoint_list_.size());
  ASSERT_EQ(hrpc::Addr("127.0.0.1", 1000), endpoint_list_.GetEndpoint(0));
  ASSERT_EQ(hrpc::Addr("127.0.0.1", 1001), endpoint_list_.GetEndpoint(1));
  endpoint_list_.PushBack({"127.0.0.1", 1002});
  endpoint_list_.PushBack({"127.0.0.1", 1003});
  endpoint_list_.PushBack({"127.0.0.1", 1004});
  ASSERT_EQ(5, endpoint_list_.size());
  ASSERT_EQ(hrpc::Addr("127.0.0.1", 1002), endpoint_list_.GetEndpoint(2));
  ASSERT_EQ(hrpc::Addr("127.0.0.1", 1003), endpoint_list_.GetEndpoint(3));
  ASSERT_EQ(hrpc::Addr("127.0.0.1", 1004), endpoint_list_.GetEndpoint(4));
  ASSERT_TRUE(!endpoint_list_.empty());
  endpoint_list_.Clear();
  ASSERT_TRUE(endpoint_list_.empty());
  endpoint_list_.PushBack({"127.0.0.1", 1000});
  endpoint_list_.PushBack({"127.0.0.1", 1001});
  endpoint_list_.PushBack({"127.0.0.1", 1002});
  endpoint_list_.PushBack({"127.0.0.1", 1003});
  endpoint_list_.PushBack({"127.0.0.1", 1004});
  ASSERT_EQ(hrpc::Addr("127.0.0.1", 1000), endpoint_list_.GetEndpoint(0));
  ASSERT_EQ(hrpc::Addr("127.0.0.1", 1001), endpoint_list_.GetEndpoint(1));
  ASSERT_EQ(hrpc::Addr("127.0.0.1", 1002), endpoint_list_.GetEndpoint(2));
  ASSERT_EQ(hrpc::Addr("127.0.0.1", 1003), endpoint_list_.GetEndpoint(3));
  ASSERT_EQ(hrpc::Addr("127.0.0.1", 1004), endpoint_list_.GetEndpoint(4));
}

TEST_F(EndpointListTest, LongList)
{
  for (size_t i = 0; i < 1000; i++) {
    endpoint_list_.PushBack({"127.0.0.1", static_cast<uint16_t>(i)});
    ASSERT_EQ(hrpc::Addr("127.0.0.1", i), endpoint_list_.GetEndpoint(i));
    if (i > 0) {
      ASSERT_EQ(hrpc::Addr("127.0.0.1", i - 1),
                endpoint_list_.GetEndpoint(i - 1));
    }
  }
}

TEST_F(EndpointListTest, CopyConstruct)
{
  for (size_t i = 0; i < 1000; i++) {
    endpoint_list_.PushBack({"127.0.0.1", static_cast<uint16_t>(i)});
  }
  hrpc::EndpointList copy(endpoint_list_);
  for (size_t i = 0; i < 1000; i++) {
    ASSERT_EQ(hrpc::Addr("127.0.0.1", i), copy.GetEndpoint(i));
  }
}

