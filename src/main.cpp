#include "pinPopup.hpp"
#include <Geode/Geode.hpp>
#include <alphalaneous.editortab_api/include/EditorTabs.hpp>
using namespace geode::prelude;
#include <Geode/modify/EditorUI.hpp>
$on_mod(Loaded)
{
	auto modGetFast = Mod::get();
	modGetFast->setSavedValue<VersionInfo>("current-version", modGetFast->getVersion());
	if (!modGetFast->getSavedValue<bool>("Patched_v1.0.3")) {
		auto savedDataJson = modGetFast->getSavedValue<std::string>("Pinned-Items");
		if (!savedDataJson.empty()) {
			std::map<std::string, bool> validIds;
			auto gm = GameManager::get();

			for (const auto &pair : matjson::parse(savedDataJson).unwrapOrDefault()) {
				if (auto str = pair.getKey()) {
					int id = std::atoi(str->c_str());
					if (id > 0 || !gm->stringForCustomObject(id).empty()) {
						validIds.insert({*str, true});
					}
				}
			}
			modGetFast->setSavedValue("Pinned-Items", matjson::Value(validIds).dump(0));
		}
		modGetFast->setSavedValue<bool>("Patched_v1.0.3",true);
	}
} // bug fix

// debug
// #define GEODE_IS_MOBILE true;

bool isValidInteger(const std::string &str) {
	if (str.empty())
		return false;

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
void ViperEditorUI_popupPrompt(int obj, EditorUI *editor);
#include <Geode/modify/CCMenuItemSpriteExtra.hpp>

class $modify(CCMenuItemSpriteExtra_MobileSelector, CCMenuItemSpriteExtra) {
	struct Fields {
		bool m_trackHolding;
		bool m_HoldAble;
		cocos2d::CCLayerColor *m_fillNode;
	};
	void popupPrompt() {
		if (this->m_fields->m_fillNode) {
			this->m_fields->m_fillNode->removeFromParentAndCleanup(true);
			this->m_fields->m_fillNode = nullptr;
		}
		if (EditorUI *editor = EditorUI::get()) {
			if (auto holding = typeinfo_cast<CreateMenuItem *>(this)) {
				ViperEditorUI_popupPrompt(holding->m_objectID, editor);
			}
		}
	}
	virtual void selected() {
		CCMenuItemSpriteExtra::selected();
		if (Mod::get()->getSettingValue<bool>("hold-to-pin") && this->m_fields->m_HoldAble && !CCScene::get()->getChildByIDRecursive("Pin-Prompt"_spr)) {
			if (auto holding = typeinfo_cast<CreateMenuItem *>(this)) {
				this->m_fields->m_trackHolding = true;
				if (this->m_fields->m_fillNode) {
					this->m_fields->m_fillNode->removeFromParentAndCleanup(true);
					this->m_fields->m_fillNode = nullptr;
				}

				this->m_fields->m_fillNode = cocos2d::CCLayerColor::create({0, 255, 0, 128}, 210.f, 35.f);
				this->m_fields->m_fillNode->setAnchorPoint({1, 0.0f});
				this->m_fields->m_fillNode->setContentSize({34, 1}); // 34
				this->m_fields->m_fillNode->setZOrder(1003);
				this->m_fields->m_fillNode->setPosition({3, 3});
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
			if (auto holding = typeinfo_cast<CreateMenuItem *>(this)) {
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
			if (auto holding = typeinfo_cast<CreateMenuItem *>(this)) {
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
#include <Geode/modify/CreateMenuItem.hpp>
class $modify(CreateMenuItem_modded, CreateMenuItem) {
	struct Fields {
		int m_customobjectid = 0;
	};
};

class $modify(ViperEditorUI, EditorUI) {
	static void onModify(auto& self) {
        (void) self.setHookPriorityAfterPost("EditorUI::init", "alphalaneous.creative_mode");
    }
	struct Fields {
		std::vector<CreateMenuItem *> m_btnfix;
		int m_objectidx = 0;
		std::string m_currentjson = "";
		CCNode* m_tooltip;
		EditButtonBar *m_createEditButtonBar;
	};
#ifdef GEODE_IS_MOBILE
	CreateMenuItem *getCreateBtn(int id, int bg) {
		CreateMenuItem *item = EditorUI::getCreateBtn(id, bg);
		if (CCMenuItemSpriteExtra *itemx = typeinfo_cast<CCMenuItemSpriteExtra *>(item)) {
			reinterpret_cast<CCMenuItemSpriteExtra_MobileSelector *>(itemx)->m_fields->m_HoldAble = true;
		}
		return item;
	}
#endif
	void selectCustomObjects(CCObject *btn) {
		if (auto b = typeinfo_cast<CreateMenuItem *>(btn)) {
			updateCreateMenu(false);
			updateGridNodeSize();
		}
	};

	void updatePinTab() {
		this->m_fields->m_currentjson = Saved::Pinned_ItemsString;
		auto rows = GameManager::get()->getIntGameVariable("0050");
		auto cols = GameManager::get()->getIntGameVariable("0049");
		this->m_fields->m_btnfix.clear();
		this->m_fields->m_createEditButtonBar->m_buttonArray->removeAllObjects();
		auto arr = this->m_fields->m_createEditButtonBar->m_buttonArray;
		arr->addObject(this->getCreateBtnViperPatch(6, 1, true));

		for (const auto &pair : matjson::parse(this->m_fields->m_currentjson).unwrapOrDefault()) {
			auto item = pair.getKey().value_or("nuUh");
			if (isValidInteger(item)) {
				if (auto btn = this->getCreateBtnViperPatch(std::stoi(item), 2)) {
					arr->addObject(btn);
				}
			}
		}
		this->m_fields->m_createEditButtonBar->loadFromItems(arr, cols, rows, true);
		Loader::get()->queueInMainThread([=]() {
			this->updateCreateMenu(false);
		});
	}
	void OnPinButton(CCObject *x) {
		pinPopup* pop = pinPopup::create([this]() {
			this->m_fields->m_currentjson = Saved::Pinned_ItemsString;
			this->updatePinTab();
			if (this->m_fields->m_tooltip) this->m_fields->m_tooltip->setVisible(false);
		});
		pop->show();
		if (this->m_fields->m_tooltip) pop->addChild(this->m_fields->m_tooltip);
	}
	void popupPrompt(int objid) {
		std::string objid_str = std::to_string(objid);
		auto Pinned_Items = matjson::parse(this->m_fields->m_currentjson).unwrapOrDefault();
		if (Pinned_Items.contains(objid_str)) {
			geode::createQuickPopup("Unpin Object?", "Are you sure you want to <cf>unpin</c> this <co>object</c>?\nYou will be unable to find it in the <cf>pinned</c> tab.", "No", "Yes", [this, objid_str](auto c, bool sel) {
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
                            return; }, true)->setID("Pin-Prompt"_spr);
		} else {
			geode::createQuickPopup("Pin Object?", "Are you sure you want to <cf>pin</c> this <co>object</c>?\nYou will be able to find it in the <cf>pinned</c> tab.", "No", "Yes", [this, objid_str](auto c, bool sel) {
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
                return; }, true)->setID("Pin-Prompt"_spr);
		}
	}
	void onCreateButton(CCObject *x) {
#ifdef GEODE_IS_MOBILE
		if (CCScene::get()->getChildByIDRecursive("Pin-Prompt"_spr)) {
			return;
		}
#endif
		if (auto xx = typeinfo_cast<CreateMenuItem *>(x)) {
			if (Mod::get()->getSettingValue<bool>("shift-keybind") && CCKeyboardDispatcher::get()->getShiftKeyPressed()) {
				return popupPrompt(xx->m_objectID);
			};
			int objid = xx->m_objectID;
			if (objid == 0) {
				objid = reinterpret_cast<CreateMenuItem_modded *>(xx)->m_fields->m_customobjectid;
			}
			if (this->m_fields->m_objectidx != objid) {
				this->m_fields->m_objectidx = objid;
			} else {
				this->m_fields->m_objectidx = 0;
			}
		};
		EditorUI::onCreateButton(x);
	};
	CreateMenuItem *menuItemFromObjectString(gd::string data, int something) {
		CreateMenuItem *custom = EditorUI::menuItemFromObjectString(data, something);
		reinterpret_cast<CreateMenuItem_modded *>(custom)->m_fields->m_customobjectid = something;
#ifdef GEODE_IS_MOBILE
		if (CCMenuItemSpriteExtra *itemx = typeinfo_cast<CCMenuItemSpriteExtra *>(custom)) {
			reinterpret_cast<CCMenuItemSpriteExtra_MobileSelector *>(itemx)->m_fields->m_HoldAble = true;
		}
#endif
		return custom;
	};
	CCNode *CreateCustomObject(int id) {
		auto gameManager = GameManager::sharedState();
		CreateMenuItem *btn = nullptr;
		std::string objectData = gameManager->stringForCustomObject(id);
		cocos2d::CCSprite *obj;
		if (!objectData.empty()) {
			btn = menuItemFromObjectString(objectData, id);
			btn->setID(fmt::format("viper.object_pinning/custom-object{}", id));
			btn->m_objectID = id;
			if (auto xd = typeinfo_cast<ButtonSprite *>(btn->getChildByType<ButtonSprite *>(0))) {
				xd->updateBGImage(Saved::switchcolor(Mod::get()->getSettingValue<std::string>("pin-objects-background")));
			};
		}
		return btn;
	};
	CCNode *getCreateBtnViperPatch(int id, int bg) {
		if (id < 1) {
			return CreateCustomObject(id);
		};
		if (id >= 4386 && id <= 4399) {
			return nullptr;
		}
		CreateMenuItem *x = EditorUI::getCreateBtn(id, Saved::switchcolorint(Mod::get()->getSettingValue<std::string>("pin-objects-background")));
		x->setID(fmt::format("viper.object_pinning/object-{}", id));
		return x;
	}
	CCNode *getCreateBtnViperPatch(int id, int bg, bool isCoolahhButton) {
		if (!isCoolahhButton) {
			return getCreateBtnViperPatch(id, bg);
		} else {
			// game sees this as pin button
			auto spr = CCMenuItemSpriteExtra::create(
			    ButtonSprite::create(CCSprite::create("1f4cc.png"_spr), 40, true, 20.f, Saved::switchcolor(Mod::get()->getSettingValue<std::string>("pin-button-background")), 1.65),
			    this,
			    menu_selector(ViperEditorUI::OnPinButton));
			spr->setID("viper.object_pinning/pin-object");
			spr->setTag(id);
			return spr;
		}
		return nullptr;
	}

	void updateCreateMenu(bool p0) {
		EditorUI::updateCreateMenu(p0);
		if (m_selectedObjectIndex != 0) {
			if (!Loader::get()->isModInstalled("razoom.object_groups")) { // mod has this bug fix
				for (auto *btn : CCArrayExt<CreateMenuItem *>(m_createButtonArray)) {
					if (btn->m_objectID == m_selectedObjectIndex) {
						ReversedGDClass::setColorToCreateBtnNew(btn, false);
					}
				}
			}
			// object groups doesn't fix custom objects
			for (auto *btn : CCArrayExt<CreateMenuItem *>(m_customObjectButtonArray)) {
				ReversedGDClass::setColorToCreateBtnNew(btn, btn->m_objectID != m_selectedObjectIndex);
			}
		}
	}
	bool init(LevelEditorLayer *editorLayer) {
		if (!EditorUI::init(editorLayer))
			return false;

		if (Loader::get()->isModInstalled("alphalaneous.creative_mode")) {
			auto creativeMode = Loader::get()->getInstalledMod("alphalaneous.creative_mode");
			if (creativeMode->isEnabled()) {
				if (CCNode* tooltip = this->getChildByID("tooltip")) this->m_fields->m_tooltip = tooltip; // creative mode support
			}
		}
		this->m_fields->m_currentjson = Mod::get()->getSavedValue<std::string>("Pinned-Items");

		EditorTabs::addTab(this /*The EditorUI*/, TabType::BUILD, "Pin"_spr, [this](EditorUI *uiOld, CCMenuItemToggler *toggler) -> CCNode * { // create the tab
			ViperEditorUI *ui = reinterpret_cast<ViperEditorUI *>(uiOld);
			auto arr = CCArray::create();
			arr->addObject(ui->getCreateBtnViperPatch(6, 1, true));

			for (const auto &pair : matjson::parse(ui->m_fields->m_currentjson).unwrapOrDefault()) {
				auto item = pair.getKey().value_or("nuUh");
				if (isValidInteger(item)) {
					if (auto btn = ui->getCreateBtnViperPatch(std::stoi(item), 2)) {
						arr->addObject(btn);
					}
				}
			}

			CCSprite *CCSPRITEON = CCSprite::create("1f4cc.png"_spr);
			CCSprite *CCSPRITEOFF = CCSprite::create("1f4cc.png"_spr);
			CCSPRITEOFF->setScale(0.8);
			CCSPRITEON->setScale(0.8);

			EditorTabUtils::setTabIcons(toggler, CCSPRITEON, CCSPRITEOFF);
			toggler->setZOrder(-999);
			this->m_fields->m_createEditButtonBar = EditorTabUtils::createEditButtonBar(arr, ui);
			return this->m_fields->m_createEditButtonBar;
		},
		                   [](EditorUI *uiOld, bool state, CCNode *) { // toggled the tab (activates on every tab click)
			                   uiOld->updateCreateMenu(false); 
		                   });

		return true;
	}
};
#ifdef GEODE_IS_MOBILE
void ViperEditorUI_popupPrompt(int obj, EditorUI *editor) {
	reinterpret_cast<ViperEditorUI *>(editor)->popupPrompt(obj);
}
#endif