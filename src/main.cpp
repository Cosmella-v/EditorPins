#include <Geode/Geode.hpp>
#include <alphalaneous.editortab_api/include/EditorTabs.hpp>
#include "pinPopup.hpp"
using namespace geode::prelude;
#include <Geode/modify/EditorUI.hpp>
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

class $modify(ViperEditorUI,EditorUI) {
    struct Fields {
        std::vector<CreateMenuItem*> m_btnfix;
        int m_objectidx = -1;
        std::string m_currentjson = "";
        EditButtonBar* m_createEditButtonBar;
    };
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
            geode::createQuickPopup("Unpin Object?","Are you sure you want to <co>unpin</c> this object?\nYou will be unable to find it in the <cf>pinned</c> tab","Yes","No",[this,objid_str](auto c, bool sel){
                            if (!sel) {
                                 auto Pinned_Items = matjson::parse(this->m_fields->m_currentjson).unwrapOrDefault();
                                if (Pinned_Items.contains(objid_str)) {
                                    Pinned_Items.erase(objid_str);
                                    Saved::Pinned_ItemsString = Pinned_Items.dump(matjson::NO_INDENTATION);
                                    this->m_fields->m_currentjson = Saved::Pinned_ItemsString;
                                    Mod::get()->setSavedValue<std::string>("Pinned-Items", Saved::Pinned_ItemsString);
                                    this->updatePinTab();
                                }
                                return;
                            }
                            return;
                        },true);
        } else {
            geode::createQuickPopup("Pin Object?","Are you sure you want to <co>pin</c> this object?\nYou will be able to find it in <cf>pinned</c> tab","Yes","No",[this,objid_str](auto c, bool sel){
                if (!sel) {
                    auto Pinned_Items = matjson::parse(this->m_fields->m_currentjson).unwrapOrDefault();
                    if (Pinned_Items.contains(objid_str)) {
                        return;
                    };
                    Pinned_Items.set(objid_str,true);
                    Saved::Pinned_ItemsString = Pinned_Items.dump(matjson::NO_INDENTATION);
                    this->m_fields->m_currentjson = Saved::Pinned_ItemsString;
                    Mod::get()->setSavedValue<std::string>("Pinned-Items", Saved::Pinned_ItemsString);
                    this->updatePinTab();
                    return;
                }
                return;
            },true);
        }
    }
    void onCreateButton(CCObject* x) {
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
        CreateMenuItem* x = EditorUI::getCreateBtn(id,bg);
        this->m_fields->m_btnfix.emplace_back(x);
        return x;
    } 
    CCNode* getCreateBtnViperPatch(int id, int bg, bool isCoolahhButton) {
        if (!isCoolahhButton) {
            return getCreateBtnViperPatch(id,bg);
        } else {
            // game sees this as pin button
            auto spr = CCMenuItemSpriteExtra::create(
                ButtonSprite::create(CCSprite::create("1f4cc.png"_spr), 40, true, 20.f,fmt::format("GJ_button_0{}.png",bg).c_str(),1), 
                this, 
                menu_selector(ViperEditorUI::OnPinButton)
            );
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
