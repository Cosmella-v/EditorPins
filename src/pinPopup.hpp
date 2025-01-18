#pragma once
#include "Reversed.hpp"
#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace Saved {
    static std::string Pinned_ItemsString = "";
};

class pinPopup : public geode::Popup<> {
protected:
    double m_minsize;
    std::function<void()> m_settings;
    ScrollLayer* scroll; 
    int maxNumber = 8; // max 8 per row
    virtual void onClose(CCObject*x) {
        Mod::get()->setSavedValue<std::string>("Pinned-Items", Saved::Pinned_ItemsString);
        m_settings();
        geode::Popup<>::onClose(x);
    };
    bool setup() {
        auto winSize = CCDirector::get()->getWinSize();

        scroll = ScrollLayer::create(ccp(
            m_mainLayer->getContentSize().width,
            m_mainLayer->getContentSize().height - 10
        ));
        scroll->setAnchorPoint(m_mainLayer->getAnchorPoint());
        scroll->setPositionY(6);
        m_mainLayer->addChild(scroll);

        return true;
    }

    bool CreateWithArgs(std::function<void()> setting) {
        Saved::Pinned_ItemsString = Mod::get()->getSavedValue<std::string>("Pinned-Items");
        auto NodeScroller = scroll->m_contentLayer;
        m_settings = setting;
        float basePosY = 150;
        int i = 1;
        int basecount = INT_MAX - 1;
        CCMenu* men = nullptr;
        auto Pinned_Items = matjson::parse(Saved::Pinned_ItemsString).unwrapOrDefault();
        for (const auto& objectName : ObjectToolbox::sharedState()->m_allKeys) {
            if (!Loader::get()->isModLoaded("geode.texture-loader")) {
                //log::info("object of death {}",objectName.first);
                if (objectName.first >= 4386 && objectName.first <= 4399) {
                    continue;
                }
            }
            basecount++;
        
            if (basecount > maxNumber) {
                basecount = 0;
                i++;
                men = CCMenu::create();
                men->setLayout(
                    RowLayout::create()
                    ->setGap(5)
                    ->setAutoScale(false)
                );
                men->setContentWidth(scroll->getContentWidth() - 30);
                men->setPosition(ccp(
                    scroll->getContentSize().width / 2,
                    basePosY - 40 * i
                ));

                NodeScroller->addChild(men);
            }
            std::string objid_str = std::to_string(objectName.first);

            auto menuItem = ReversedGDClass::getCreateBtn(objectName.first,4); 
            if (Pinned_Items.contains(objid_str)) {
                if (auto xd = typeinfo_cast<ButtonSprite*>(menuItem->getNormalImage())) {
                    xd->m_subBGSprite->setColor(ccColor3B(0,255,0));
                };
            } 
            menuItem->m_pfnSelector = menu_selector(pinPopup::OnPinButton);
            /*EditorButtonSprite::createWithSpriteFrameName(
                 GameObeobjectName.second.c_str(),
                 EditorBaseColor::Gray,
                 EditorBaseSize::Normal
            );
            auto menuItem = CCMenuItemSpriteExtra::create(
                button,
                button,
                menu_selector(pinPopup::OnPinButton)
            );*/
            men->addChild(menuItem);
            men->updateLayout();
        }

        m_minsize = scroll->getContentSize().height;
        float height = std::max<float>(m_minsize, 40 * i);
        
        NodeScroller->setContentSize(ccp(NodeScroller->getContentSize().width, height));
        CCArrayExt<CCNode*> objects = NodeScroller->getChildren(); // fix me
        i = 0;
        for (auto* obj : objects) {
            i++;
            obj->setPositionY(height - (40 * i));
        }

        scroll->moveToTop();

        return true;
    }
    bool JsonChange(int objid) {
        std::string objid_str = std::to_string(objid);
        auto Pinned_Items = matjson::parse(Saved::Pinned_ItemsString).unwrapOrDefault();
        if (Pinned_Items.contains(objid_str)) {
            Pinned_Items.erase(objid_str);
            Saved::Pinned_ItemsString = Pinned_Items.dump(matjson::NO_INDENTATION);
            return false;
        }

        Pinned_Items.set(objid_str,true);
        Saved::Pinned_ItemsString = Pinned_Items.dump(matjson::NO_INDENTATION);
        return true;
    }
    void OnPinButton(CCObject* sender) {
        // Handle button press logic here
        if (CreateMenuItem* Item = typeinfo_cast<CreateMenuItem*>(sender)) {
            if (auto xd = typeinfo_cast<ButtonSprite*>(Item->getNormalImage())) {
                if (JsonChange(Item->m_objectID) ) {
                   xd->m_subBGSprite->setColor(ccColor3B(0,255,0));
                } else {
                    xd->m_subBGSprite->setColor(ccColor3B(255,255,255));
                }
            }
        };
    }

public:
    static pinPopup* create(std::function<void()> setting = []() {}) {
        auto ret = new pinPopup;

        if (ret && ret->initAnchored(420.f, 270.f, "GJ_square05.png")) {
            ret->autorelease();
            ret->CreateWithArgs(setting);
            ret->setID("PinPopup"_spr);
            return ret;
        }

        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};
