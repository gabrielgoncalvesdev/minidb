
#include "minidb/common/config.hpp"
#include "minidb/storage/page/page.hpp"
#include "minidb/storage/page/page_guard.hpp"


#include <utility>
#include <cstring>
#include <filesystem>
#include <string>

#include <gtest/gtest.h>

#include "minidb/buffer/buffer_pool_manager.hpp"
#include "minidb/storage/disk/disk_manager.hpp"

namespace minidb {
    namespace {
        class PageGuardTest : public ::testing::Test {
            protected:
            void SetUp() override {
                path_ = (std::filesystem::temp_directory_path()/"minidb_test_guard.db").string();
                std::filesystem::remove(path_);
            }
            void TearDown() override { std::filesystem::remove(path_); }
                std::string path_;
        };
        TEST_F(PageGuardTest, AutoUnpinsOnScopeExit) {
            DiskManager dm(path_);
            BufferPoolManager bpm(3, &dm, 2);
            page_id_t id = INVALID_PAGE_ID;
            Page* p = bpm.NewPage(&id);
            ASSERT_NE(p, nullptr);
            EXPECT_EQ(p->PinCount(), 1);
            {
                PageGuard guard(&bpm, p);
                EXPECT_EQ(p->PinCount(), 1);
            }
            EXPECT_EQ(p->PinCount(), 0);
        }

        TEST_F(PageGuardTest, DataMutMarksDirtyAndPersists) {
            DiskManager dm(path_);
            BufferPoolManager bpm(3, &dm, 2);
            const char* msg = "guarded";
            page_id_t id = INVALID_PAGE_ID;
            {
                Page* p = bpm.NewPage(&id);
                ASSERT_NE(p, nullptr);
                PageGuard g(&bpm, p);
                std::memcpy(g.DataMut(), msg, std::strlen(msg) + 1);
            } 
            page_id_t other = INVALID_PAGE_ID;
            {
                Page* q = bpm.NewPage(&other);
                ASSERT_NE(q, nullptr);
                PageGuard g2(&bpm, q);
            }
            Page* again = bpm.FetchPage(id);
            ASSERT_NE(again, nullptr);
            EXPECT_STREQ(reinterpret_cast<const char*>(again->Data()), msg);
            EXPECT_TRUE(bpm.UnpinPage(id, false));
        }

    }
}

