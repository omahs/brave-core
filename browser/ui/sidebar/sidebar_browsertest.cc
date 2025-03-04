/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "base/test/scoped_feature_list.h"
#include "brave/browser/ui/brave_browser.h"
#include "brave/browser/ui/sidebar/sidebar_controller.h"
#include "brave/browser/ui/sidebar/sidebar_model.h"
#include "brave/browser/ui/sidebar/sidebar_service_factory.h"
#include "brave/browser/ui/sidebar/sidebar_utils.h"
#include "brave/browser/ui/views/frame/brave_browser_view.h"
#include "brave/browser/ui/views/sidebar/sidebar_container_view.h"
#include "brave/browser/ui/views/sidebar/sidebar_control_view.h"
#include "brave/browser/ui/views/sidebar/sidebar_items_contents_view.h"
#include "brave/browser/ui/views/sidebar/sidebar_items_scroll_view.h"
#include "brave/browser/ui/views/tabs/features.h"
#include "brave/components/sidebar/sidebar_service.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "chrome/common/pref_names.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "chrome/test/base/ui_test_utils.h"
#include "components/prefs/pref_service.h"
#include "content/public/test/browser_test.h"
#include "testing/gmock/include/gmock/gmock-matchers.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "ui/events/base_event_utils.h"
#include "ui/events/event.h"
#include "ui/gfx/geometry/point.h"

using ::testing::Eq;
using ::testing::Optional;

namespace sidebar {

class SidebarBrowserTest : public InProcessBrowserTest {
 public:
  SidebarBrowserTest() = default;
  ~SidebarBrowserTest() override = default;

  void PreRunTestOnMainThread() override {
    InProcessBrowserTest::PreRunTestOnMainThread();

    auto* service = SidebarServiceFactory::GetForProfile(browser()->profile());
    // Enable sidebar explicitely because sidebar option is different based on
    // channel.
    service->SetSidebarShowOption(
        SidebarService::ShowSidebarOption::kShowAlways);
  }

  BraveBrowser* brave_browser() const {
    return static_cast<BraveBrowser*>(browser());
  }

  SidebarModel* model() const { return controller()->model(); }
  TabStripModel* tab_model() const { return browser()->tab_strip_model(); }

  SidebarController* controller() const {
    return brave_browser()->sidebar_controller();
  }

  views::View* GetVerticalTabsContainer() const {
    auto* view = BrowserView::GetBrowserViewForBrowser(browser());
    return static_cast<BraveBrowserView*>(view)->vertical_tab_strip_host_view_;
  }

  views::Widget* GetEventDetectWidget() {
    auto* sidebar_container_view =
        static_cast<SidebarContainerView*>(controller()->sidebar());
    return sidebar_container_view->GetEventDetectWidget()->widget_.get();
  }

  void SimulateSidebarItemClickAt(int index) const {
    auto* sidebar_container_view =
        static_cast<SidebarContainerView*>(controller()->sidebar());
    auto sidebar_control_view = sidebar_container_view->sidebar_control_view_;
    auto sidebar_scroll_view = sidebar_control_view->sidebar_items_view_;
    auto sidebar_items_contents_view = sidebar_scroll_view->contents_view_;
    DCHECK(sidebar_items_contents_view);

    auto* item = sidebar_items_contents_view->children()[index];
    DCHECK(item);

    const gfx::Point origin(0, 0);
    ui::MouseEvent event(ui::ET_MOUSE_PRESSED, origin, origin,
                         ui::EventTimeForNow(), ui::EF_LEFT_MOUSE_BUTTON, 0);
    sidebar_items_contents_view->OnItemPressed(item, event);
  }

  bool IsSidebarUIOnLeft() const {
    auto* sidebar_container_view =
        static_cast<SidebarContainerView*>(controller()->sidebar());
    return sidebar_container_view->sidebar_on_left_;
  }
};

IN_PROC_BROWSER_TEST_F(SidebarBrowserTest, BasicTest) {
  // Initially, active index is not set.
  EXPECT_THAT(model()->active_index(), Eq(absl::nullopt));

  // Check sidebar UI is initalized properly.
  EXPECT_TRUE(!!controller()->sidebar());

  // Currently we have 4 default items.
  EXPECT_EQ(4UL, model()->GetAllSidebarItems().size());
  // Activate item that opens in panel.
  controller()->ActivateItemAt(2);
  EXPECT_THAT(model()->active_index(), Optional(2u));
  EXPECT_TRUE(controller()->IsActiveIndex(2));

  // Try to activate item at index 1.
  // Default item at index 1 opens in new tab. So, sidebar active index is not
  // changed. Still active index is 2.
  const auto item = model()->GetAllSidebarItems()[1];
  EXPECT_FALSE(item.open_in_panel);
  controller()->ActivateItemAt(1);
  EXPECT_THAT(model()->active_index(), Optional(2u));

  // Setting absl::nullopt means deactivate current active tab.
  controller()->ActivateItemAt(absl::nullopt);
  EXPECT_THAT(model()->active_index(), Eq(absl::nullopt));

  controller()->ActivateItemAt(2);

  // Remove Item at index 0 change active index from 3 to 2.
  SidebarServiceFactory::GetForProfile(browser()->profile())->RemoveItemAt(0);
  EXPECT_EQ(3UL, model()->GetAllSidebarItems().size());
  EXPECT_THAT(model()->active_index(), Optional(1u));

  // If current active tab is not NTP, we can add current url to sidebar.
  EXPECT_TRUE(CanAddCurrentActiveTabToSidebar(browser()));

  // If current active tab is NTP, we can't add current url to sidebar.
  ASSERT_TRUE(ui_test_utils::NavigateToURL(browser(), GURL("brave://newtab/")));
  EXPECT_FALSE(CanAddCurrentActiveTabToSidebar(browser()));

  // Check |BrowserView::find_bar_host_view_| is the last child view.
  // If not, findbar dialog is not positioned properly.
  BrowserView* browser_view = BrowserView::GetBrowserViewForBrowser(browser());
  auto find_bar_host_view_index =
      browser_view->GetIndexOf(browser_view->find_bar_host_view());
  EXPECT_THAT(find_bar_host_view_index,
              Optional(browser_view->children().size() - 1));
}

IN_PROC_BROWSER_TEST_F(SidebarBrowserTest, WebTypePanelTest) {
  // By default, sidebar has 4 items.
  EXPECT_EQ(4UL, model()->GetAllSidebarItems().size());

  // Add an item
  ASSERT_TRUE(
      ui_test_utils::NavigateToURL(browser(), GURL("brave://settings/")));
  int current_tab_index = tab_model()->active_index();
  EXPECT_EQ(0, current_tab_index);
  EXPECT_TRUE(CanAddCurrentActiveTabToSidebar(browser()));
  controller()->AddItemWithCurrentTab();
  // Verify new size
  EXPECT_EQ(5UL, model()->GetAllSidebarItems().size());

  // Load NTP in a new tab and activate it. (tab index 1)
  ASSERT_TRUE(ui_test_utils::NavigateToURLWithDisposition(
      browser(), GURL("brave://newtab/"),
      WindowOpenDisposition::NEW_FOREGROUND_TAB,
      ui_test_utils::BROWSER_TEST_WAIT_FOR_LOAD_STOP));
  current_tab_index = tab_model()->active_index();
  EXPECT_EQ(1, current_tab_index);

  // Activate sidebar item(brave://settings) and check existing first tab is
  // activated.
  auto item = model()->GetAllSidebarItems()[4];
  controller()->ActivateItemAt(4);
  EXPECT_EQ(0, tab_model()->active_index());
  EXPECT_EQ(tab_model()->GetWebContentsAt(0)->GetVisibleURL(), item.url);

  // Activate second sidebar item(wallet) and check it's loaded at current tab.
  item = model()->GetAllSidebarItems()[1];
  controller()->ActivateItemAt(1);
  EXPECT_EQ(0, tab_model()->active_index());
  EXPECT_EQ(tab_model()->GetWebContentsAt(0)->GetVisibleURL(), item.url);
  // New tab is not created.
  EXPECT_EQ(2, tab_model()->count());
}

IN_PROC_BROWSER_TEST_F(SidebarBrowserTest, IterateBuiltInWebTypeTest) {
  // Click builtin wallet item and it's loaded at current active tab.
  auto item = model()->GetAllSidebarItems()[1];
  EXPECT_FALSE(controller()->DoesBrowserHaveOpenedTabForItem(item));
  SimulateSidebarItemClickAt(1);
  EXPECT_TRUE(controller()->DoesBrowserHaveOpenedTabForItem(item));
  EXPECT_EQ(0, tab_model()->active_index());
  EXPECT_EQ(tab_model()->GetWebContentsAt(0)->GetVisibleURL().host(),
            item.url.host());

  // Create NTP and click wallet item. Then wallet tab(index 0) is activated.
  ASSERT_TRUE(ui_test_utils::NavigateToURLWithDisposition(
      browser(), GURL("brave://newtab/"),
      WindowOpenDisposition::NEW_FOREGROUND_TAB,
      ui_test_utils::BROWSER_TEST_WAIT_FOR_LOAD_STOP));
  // NTP is active tab.
  EXPECT_EQ(1, tab_model()->active_index());
  SimulateSidebarItemClickAt(1);
  // Wallet tab is active tab.
  EXPECT_EQ(0, tab_model()->active_index());
  EXPECT_EQ(tab_model()->GetWebContentsAt(0)->GetVisibleURL().host(),
            item.url.host());

  // Create NTP.
  ASSERT_TRUE(ui_test_utils::NavigateToURLWithDisposition(
      browser(), GURL("brave://newtab/"),
      WindowOpenDisposition::NEW_FOREGROUND_TAB,
      ui_test_utils::BROWSER_TEST_WAIT_FOR_LOAD_STOP));
  // NTP is active tab and load wallet on it.
  EXPECT_EQ(2, tab_model()->active_index());
  ASSERT_TRUE(ui_test_utils::NavigateToURL(browser(), item.url));

  // Click wallet item and then first wallet tab(tab index 0) is activated.
  SimulateSidebarItemClickAt(1);
  EXPECT_EQ(0, tab_model()->active_index());

  // Click wallet item and then second wallet tab(index 2) is activated.
  SimulateSidebarItemClickAt(1);
  EXPECT_EQ(2, tab_model()->active_index());

  // Click wallet item and then first wallet tab(index 0) is activated.
  SimulateSidebarItemClickAt(1);
  EXPECT_EQ(0, tab_model()->active_index());
}

IN_PROC_BROWSER_TEST_F(SidebarBrowserTest, EventDetectWidgetTest) {
  auto* widget = GetEventDetectWidget();
  auto* service = SidebarServiceFactory::GetForProfile(browser()->profile());
  auto* browser_view = BrowserView::GetBrowserViewForBrowser(browser());
  auto* contents_container = browser_view->contents_container();
  auto* prefs = browser()->profile()->GetPrefs();

  // Check widget is located on left side when sidebar on left.
  prefs->SetBoolean(prefs::kSidePanelHorizontalAlignment, false);
  service->SetSidebarShowOption(
      SidebarService::ShowSidebarOption::kShowOnMouseOver);
  EXPECT_EQ(contents_container->GetBoundsInScreen().x(),
            widget->GetWindowBoundsInScreen().x());

  // Check widget is located on right side when sidebar on right.
  prefs->SetBoolean(prefs::kSidePanelHorizontalAlignment, true);
  EXPECT_EQ(contents_container->GetBoundsInScreen().right(),
            widget->GetWindowBoundsInScreen().right());
}

class SidebarBrowserTestWithVerticalTabs : public SidebarBrowserTest {
 public:
  SidebarBrowserTestWithVerticalTabs() {
    feature_list_.InitAndEnableFeature(tabs::features::kBraveVerticalTabs);
  }
  ~SidebarBrowserTestWithVerticalTabs() override = default;

  base::test::ScopedFeatureList feature_list_;
};

IN_PROC_BROWSER_TEST_F(SidebarBrowserTestWithVerticalTabs,
                       SidebarRightSideTest) {
  auto* prefs = browser()->profile()->GetPrefs();
  auto* browser_view = BrowserView::GetBrowserViewForBrowser(browser());

  auto* vertical_tabs_container = GetVerticalTabsContainer();
  auto* sidebar_container =
      static_cast<SidebarContainerView*>(controller()->sidebar());

  // Sidebar is on left.
  EXPECT_TRUE(IsSidebarUIOnLeft());

  // Check vertical tabs is located right after sidebar.
  EXPECT_EQ(sidebar_container->bounds().right(), vertical_tabs_container->x());

  // Changed to sidebar on right side.
  prefs->SetBoolean(prefs::kSidePanelHorizontalAlignment, true);
  EXPECT_FALSE(IsSidebarUIOnLeft());

  // Check vertical tabs is located at first.
  EXPECT_EQ(0, vertical_tabs_container->x());

  // Check sidebar is located on the right side.
  EXPECT_EQ(sidebar_container->bounds().right(), browser_view->width());

  // Changed to sidebar on left side again.
  prefs->SetBoolean(prefs::kSidePanelHorizontalAlignment, false);
  EXPECT_TRUE(IsSidebarUIOnLeft());

  // Check vertical tabs is located right after sidebar.
  EXPECT_EQ(sidebar_container->bounds().right(), vertical_tabs_container->x());
}

}  // namespace sidebar
