#include <Geode/Geode.hpp>
#include <alphalaneous.editortab_api/include/EditorTabs.hpp>
#include "pinPopup.hpp"
using namespace geode::prelude;
#include <Geode/modify/EditorUI.hpp>

// debug
//#define GEODE_IS_MOBILE true;

bool isValidInteger(const std::string& str) {
    if (str.empty()) return false;
    
    size_t start = 0;
    if (str[0] == '-' || str[0] == '+') {
        start = 1; 
    }
    
    for (size_t i = start; i < str.length(); ++i) {
        if (!std::isdigit(str[i])) {
            return false; 
        }
    }
    
    return true;
}
// android features
#ifdef GEODE_IS_MOBILE
void ViperEditorUI_popupPrompt(int obj, EditorUI* editor);
#include <Geode/modify/CCMenuItemSpriteExtra.hpp>

class $modify(CreateMenuItem_MobileSelector,CCMenuItemSpriteExtra) {
struct Fields {
    bool m_trackHolding;
    bool m_HoldAble;
    cocos2d::CCLayerColor* m_fillNode;
};
void popupPrompt() {
    if (this->m_fields->m_fillNode) {
        this->m_fields->m_fillNode->removeFromParentAndCleanup(true);
        this->m_fields->m_fillNode = nullptr;
    }
    if (EditorUI* editor = EditorUI::get()) {
        if (auto holding = typeinfo_cast<CreateMenuItem*>(this)) {
            ViperEditorUI_popupPrompt(holding->m_objectID, editor);
        }
    }
}
virtual void selected() {
    CCMenuItemSpriteExtra::selected();
    if (Mod::get()->getSettingValue<bool>("hold-to-pin") && this->m_fields->m_HoldAble && !CCScene::get()->getChildByIDRecursive("Pin-Prompt"_spr)) {
    if (auto holding = typeinfo_cast<CreateMenuItem*>(this)) {
        this->m_fields->m_trackHolding = true;
        if (this->m_fields->m_fillNode) {
            this->m_fields->m_fillNode->removeFromParentAndCleanup(true);
            this->m_fields->m_fillNode = nullptr;
        }
        
        this->m_fields->m_fillNode = cocos2d::CCLayerColor::create({0, 255, 0, 128},210.f, 35.f); 
        this->m_fields->m_fillNode->setAnchorPoint({1, 0.0f}); 
        this->m_fields->m_fillNode->setContentSize({34,1}); // 34 
        this->m_fields->m_fillNode->setZOrder(1003); 
        this->m_fields->m_fillNode->setPosition({3,3});
        this->m_fields->m_fillNode->setID("fill-bar"_spr);
        auto fillAction = CCScaleTo::create(2.5, 1, 34);
        this->m_pNormalImage->addChild(this->m_fields->m_fillNode);
        auto callFuncAction = CCCallFunc::create(this, callfunc_selector(CCMenuItemSpriteExtra_MobileSelector::popupPrompt));
        auto sequence = CCSequence::create(fillAction, callFuncAction, nullptr); 
        this->m_fields->m_fillNode->runAction(sequence);
        }
    }
}
virtual void activate() {
    if (this->m_fields->m_HoldAble) {
        if (auto holding = typeinfo_cast<CreateMenuItem*>(this)) {
            this->m_fields->m_trackHolding = false;
            if (this->m_fields->m_fillNode) {
                this->m_fields->m_fillNode->removeFromParentAndCleanup(true);
                this->m_fields->m_fillNode = nullptr;
            }
        }
    }
    CCMenuItemSpriteExtra::activate();
}
virtual void unselected() {
    if (this->m_fields->m_HoldAble) {
        if (auto holding = typeinfo_cast<CreateMenuItem*>(this)) {
            this->m_fields->m_trackHolding = false;
            if (this->m_fields->m_fillNode) {
                this->m_fields->m_fillNode->removeFromParentAndCleanup(true);
                this->m_fields->m_fillNode = nullptr;
            }
        }
    }
    CCMenuItemSpriteExtra::unselected();
}

};
#endif

/*
"hide-pin-button": {
			"name": "Disable Pin button",
			"description": "Disables the <cf>pin</c> button to view all objects",
			"type": "bool",
			"enable-if": "!shift-keybind && !hold-to-pin",
			"default": false
		},
        
*/
class $modify(ViperEditorUI,EditorUI) {
    struct Fields {
        std::vector<CreateMenuItem*> m_btnfix;
        int m_objectidx = -1;
        std::string m_currentjson = "";
        EditButtonBar* m_createEditButtonBar;
    };
    #ifdef GEODE_IS_MOBILE
    CreateMenuItem* getCreateBtn(int id, int bg) {
        CreateMenuItem* item = EditorUI::getCreateBtn(id,bg);
        if (CCMenuItemSpriteExtra* itemx = typeinfo_cast<CCMenuItemSpriteExtra*>(item)) {
            reinterpret_cast<CCMenuItemSpriteExtra_MobileSelector*>(itemx)->m_fields->m_HoldAble = true;
        }
        return item;
    }
    #endif
    void updatePinTab(){
            this->m_fields->m_currentjson = Saved::Pinned_ItemsString;
            auto rows = GameManager::get()->getIntGameVariable("0050");
            auto cols = GameManager::get()->getIntGameVariable("0049");
            this->m_fields->m_btnfix.clear();
            this->m_fields->m_createEditButtonBar->m_buttonArray->removeAllObjects();
            auto arr = this->m_fields->m_createEditButtonBar->m_buttonArray;
            arr->addObject(this->getCreateBtnViperPatch(6,1,true));

             for (const auto& pair :  matjson::parse(this->m_fields->m_currentjson).unwrapOrDefault() ) {
                auto item = pair.getKey().value_or("nuUh");
                if (isValidInteger(item)) {
                    arr->addObject(this->getCreateBtnViperPatch(std::stoi(item),2));
                }
			}
            this->m_fields->m_createEditButtonBar->loadFromItems(arr,cols,rows,true);
            Loader::get()->queueInMainThread([=](){
                this->fixsprite_fix();
            });
    }
    void OnPinButton(CCObject* x) {
        pinPopup::create([this]() {
            this->m_fields->m_currentjson = Saved::Pinned_ItemsString;
            this->updatePinTab();
        })->show();
    }
    void popupPrompt(int objid) {
        std::string objid_str = std::to_string(objid);
        auto Pinned_Items = matjson::parse(this->m_fields->m_currentjson).unwrapOrDefault();
        if (Pinned_Items.contains(objid_str)) {
            geode::createQuickPopup("Unpin Object?","Are you sure you want to <cf>unpin</c> this <co>object</c>?\nYou will be unable to find it in the <cf>pinned</c> tab.","No","Yes",[this,objid_str](auto c, bool sel){
                            if (sel) {
                                 auto Pinned_Items = matjson::parse(this->m_fields->m_currentjson).unwrapOrDefault();
                                if (Pinned_Items.contains(objid_str)) {
                                    Pinned_Items.erase(objid_str);
                                    Saved::Pinned_ItemsString = Pinned_Items.dump(matjson::NO_INDENTATION);
                                    Mod::get()->setSavedValue<std::string>("Pinned-Items", Saved::Pinned_ItemsString);
                                    this->updatePinTab();
                                }
                                return;
                            }
                            return;
                        },true)->setID("Pin-Prompt"_spr);
        } else {
            geode::createQuickPopup("Pin Object?","Are you sure you want to <cf>pin</c> this <co>object</c>?\nYou will be able to find it in the <cf>pinned</c> tab.","No","Yes",[this,objid_str](auto c, bool sel){
                if (sel) {
                    auto Pinned_Items = matjson::parse(this->m_fields->m_currentjson).unwrapOrDefault();
                    if (Pinned_Items.contains(objid_str)) {
                        return;
                    };
                    Pinned_Items.set(objid_str,true);
                    Saved::Pinned_ItemsString = Pinned_Items.dump(matjson::NO_INDENTATION);
                    Mod::get()->setSavedValue<std::string>("Pinned-Items", Saved::Pinned_ItemsString);
                    this->updatePinTab();
                    return;
                }
                return;
            },true)->setID("Pin-Prompt"_spr);
        }
    }
    void onCreateButton(CCObject* x) {
        #ifdef GEODE_IS_MOBILE
        if (CCScene::get()->getChildByIDRecursive("Pin-Prompt"_spr)) {
            return;
        }
        #endif
        if (auto xx =  typeinfo_cast<CreateMenuItem*>(x)) {
                if (xx->m_objectID > 0 && Mod::get()->getSettingValue<bool>("shift-keybind") && CCKeyboardDispatcher::get()->getShiftKeyPressed()) {
                    return popupPrompt(xx->m_objectID);
                };
                if (this->m_fields->m_objectidx != xx->m_objectID) {
                    this->m_fields->m_objectidx = xx->m_objectID;
                } else {
                    this->m_fields->m_objectidx = -1;
                }
        };
        EditorUI::onCreateButton(x);
        return fixsprite_fix();
    };
    void fixsprite_fix() {
        for (size_t i = 0; i < this->m_fields->m_btnfix.size(); ++i) {
            //std::cout << "Index " << i << ": " << this->m_fields->m_btnfix[i] << "\n";
            //log::debug("log");
            CreateMenuItem* m_btnfixs = this->m_fields->m_btnfix.at(i);
            if (auto xd = typeinfo_cast<ButtonSprite*>(m_btnfixs->getNormalImage())) {
                if (m_btnfixs->m_objectID == this->m_fields->m_objectidx ) {
                    xd->m_subBGSprite->setColor(ccColor3B(177,177,177));
                } else {
                    xd->m_subBGSprite->setColor(ccColor3B(255,255,255));
                    //log::debug("log3");
                }
                if (m_btnfixs->m_objectID  >= 4386 && m_btnfixs->m_objectID  <= 4399) {
                    if (GameObject* object = xd->getChildByType<GameObject*>(0)) {
                        object->setScale(0.175);
                    };
                };
            } else {log::error("something bad got in");}
        };
    }
    CCNode* getCreateBtnViperPatch(int id, int bg) {
        if (id < 1) {
            auto cc = CCNode::create();
            cc->autorelease();
            return cc;
        };
         if (!Loader::get()->isModLoaded("geode.texture-loader")) {
            if (id >= 4386 && id <= 4399) {
                auto cc = CCNode::create();
                cc->autorelease();
                return cc;
            }
        };
        CreateMenuItem* x = EditorUI::getCreateBtn(id,Saved::switchcolorint(Mod::get()->getSettingValue<std::string>("pin-objects-background")));
        this->m_fields->m_btnfix.emplace_back(x);
        x->setID(fmt::format("viper.object_pinning/object-{}",id));
        return x;
    } 
    CCNode* getCreateBtnViperPatch(int id, int bg, bool isCoolahhButton) {
        if (!isCoolahhButton) {
            return getCreateBtnViperPatch(id,bg);
        } else {
            // game sees this as pin button
            auto spr = CCMenuItemSpriteExtra::create(
                ButtonSprite::create(CCSprite::create("1f4cc.png"_spr), 40, true, 20.f,Saved::switchcolor(Mod::get()->getSettingValue<std::string>("pin-button-background")),1.65), 
                this, 
                menu_selector(ViperEditorUI::OnPinButton)
            );
            spr->setID("viper.object_pinning/pin-object");
            spr->setTag(id);
            return spr;
        }
        return nullptr;
    } 
    bool init(LevelEditorLayer* editorLayer) {
        if (!EditorUI::init(editorLayer)) return false;

        this->m_fields->m_currentjson = Mod::get()->getSavedValue<std::string>("Pinned-Items");

        EditorTabs::addTab(this /*The EditorUI*/, TabType::BUILD, "Pin"_spr, [this](EditorUI* uiOld, CCMenuItemToggler* toggler) -> CCNode* { //create the tab
            ViperEditorUI* ui = reinterpret_cast<ViperEditorUI*>(uiOld);
            auto arr = CCArray::create();
            arr->addObject(ui->getCreateBtnViperPatch(6,1,true));

             for (const auto& pair :  matjson::parse(ui->m_fields->m_currentjson).unwrapOrDefault() ) {
                auto item = pair.getKey().value_or("nuUh");
                if (isValidInteger(item)) {
                    arr->addObject(ui->getCreateBtnViperPatch(std::stoi(item),2));
                }
			}

            CCSprite* CCSPRITEON =  CCSprite::create("1f4cc.png"_spr);
            CCSprite* CCSPRITEOFF = CCSprite::create("1f4cc.png"_spr);
            CCSPRITEOFF->setScale(0.8);
            CCSPRITEON->setScale(0.8);

            EditorTabUtils::setTabIcons(toggler, CCSPRITEON, CCSPRITEOFF);
            toggler->setZOrder(-999);
            this->m_fields->m_createEditButtonBar = EditorTabUtils::createEditButtonBar(arr, ui);
            return this->m_fields->m_createEditButtonBar;
        }, [](EditorUI* uiOld, bool state, CCNode*) { //toggled the tab (activates on every tab click)
            ViperEditorUI* ui = reinterpret_cast<ViperEditorUI*>(uiOld);
            ui->fixsprite_fix(); // bug fix
        });

        return true;
    }
};
#ifdef GEODE_IS_MOBILE
void ViperEditorUI_popupPrompt(int obj, EditorUI* editor) {
   reinterpret_cast<ViperEditorUI*>(editor)->popupPrompt(obj);
}
#endif