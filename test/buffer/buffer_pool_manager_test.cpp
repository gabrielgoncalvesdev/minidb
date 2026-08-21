#include "minidb/buffer/buffer_pool_manager.hpp"
#include "minidb/common/config.hpp"
#include "minidb/storage/disk/disk_manager.hpp"

#include <cstddef>
#include <cstring>
#include <filesystem>
#include <string>

#include <gtest/gtest.h>



namespace minidb {
    namespace {
        class BufferPoolManagerTest : public ::testing::Test {
            protected:
            void SetUp() override {
                path_ = (std::filesystem::temp_directory_path() / "minidb_bpm_test.db").string();
                std::filesystem::remove(path_);
            }
            void TearDown() override { std::filesystem::remove(path_); }
            std::string path_;
        };

        TEST_F(BufferPoolManagerTest, NewPageAssignsSequentialIds) {
            DiskManager dm(path_);
            BufferPoolManager bpm(10, &dm, 2);

            page_id_t id0 = INVALID_PAGE_ID;
            page_id_t id1 = INVALID_PAGE_ID;
            Page* p0 = bpm.NewPage(&id0);
            Page* p1 = bpm.NewPage(&id1);
            ASSERT_NE(p0, nullptr);
            ASSERT_NE(p1,  nullptr);
            EXPECT_EQ(id0, 0);
            EXPECT_EQ(id1, 1);
            EXPECT_EQ(p0->PinCount(), 1);
        }
        
        TEST_F(BufferPoolManagerTest, PoolFillsUpThenFailsUntilUpin ) {
            DiskManager dm(path_);
            const std::size_t pool = 3;
            BufferPoolManager bpm(pool, &dm, 2);

            page_id_t id = INVALID_PAGE_ID;
            for (std::size_t i = 0; i < pool; ++i) {
                ASSERT_NE(bpm.NewPage(&id), nullptr);
            }
            EXPECT_EQ(bpm.NewPage(&id), nullptr);

            EXPECT_TRUE(bpm.UnpinPage(0, false));
            EXPECT_NE(&id, nullptr);
        }

        TEST_F(BufferPoolManagerTest, DataPersistsThroughEviction) {
            DiskManager dm(path_);
            BufferPoolManager bpm(1, &dm, 2);

            page_id_t target = INVALID_PAGE_ID;
            Page* p = bpm.NewPage(&target);
            ASSERT_NE(p, nullptr);
            const char* msg = "hello minidb";
            std::memcpy(p->Data(), msg, std::strlen(msg) + 1 );
            EXPECT_TRUE(bpm.UnpinPage(target, true)); //alterado

            page_id_t other = INVALID_PAGE_ID;
            ASSERT_NE(bpm.NewPage(&other), nullptr);
            EXPECT_TRUE(bpm.UnpinPage(other, false));

            Page* again = bpm.FetchPage(target);
            ASSERT_NE(again, nullptr);
            EXPECT_STREQ(reinterpret_cast<const char*>(again->Data()), msg);
        }

        TEST_F(BufferPoolManagerTest, DeletePageFreesFrame) {
            DiskManager dm(path_);
            BufferPoolManager bpm(2, &dm, 2);
            page_id_t id = INVALID_PAGE_ID;
            Page* p = bpm.NewPage(&id);
            ASSERT_NE(p, nullptr);
            EXPECT_FALSE(bpm.DeletePage(id)); //fixada não deve deletar
            EXPECT_TRUE(bpm.UnpinPage(id, false));
            EXPECT_TRUE(bpm.DeletePage(id));
        }

    }
}