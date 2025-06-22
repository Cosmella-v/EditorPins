#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace ReversedGDClass {

static void setColorToGameObjectNew(GameObject *gameObj, bool isBright) {
	ccColor3B color = isBright ? ccc3(255, 255, 255) : ccc3(127, 127, 127);
	int objId;
	if ((int)gameObj->m_classType == 1) {
		bool cVar14;
		if (gameObj->m_customColorType == 0) {
			cVar14 = gameObj->m_maybeNotColorable;
		} else {
			cVar14 = (gameObj->m_customColorType == 1);
		}
		if (cVar14 || gameObj->m_colorSprite ||
		    gameObj->m_baseColor->m_defaultColorID == 0x3ec ||
		    gameObj->m_baseColor->m_defaultColorID == 0x0 ||
		    /* (*(char *)(gameObj + 0xdf) == '\0') */ false) {
			goto LAB_14010da56;
		}
		color = isBright ? ccc3(200, 200, 255) : ccc3(100, 100, 127);
		goto LAB_14010dac2;
	}
LAB_14010da56:
	objId = gameObj->m_objectID;
	bool bVar12;
	if (objId < 0x531) {
		if (((objId == 0x530) || (objId == 0x396)) || (objId == 0x397))
			goto LAB_14010daad;
		bVar12 = (objId == 0x52f);
	LAB_14010da89:
		if (bVar12)
			goto LAB_14010daad;
		auto piVar3 = gameObj->m_baseColor;
		if (piVar3 != 0) {
			objId = piVar3->m_colorID;
			if ((piVar3->m_defaultColorID == objId) || (objId == 0x0)) {
				objId = piVar3->m_defaultColorID;
			}
			if (objId == 0x3f2)
				goto LAB_14010daad;
		}
	} else {
		if (objId != 0x630) {
			bVar12 = (objId == 0x7dc);
			goto LAB_14010da89;
		}
	LAB_14010daad:
		color = isBright ? ccc3(0, 0, 0) : ccc3(127, 127, 127);
	}
LAB_14010dac2:
	gameObj->setObjectColor(color);
	color = isBright ? ccc3(200, 200, 255) : ccc3(100, 100, 127);
	gameObj->setChildColor(color);
}

static void setColorToCreateBtnNew(CreateMenuItem *cmi, bool isBright) {
	ccColor3B color = isBright ? ccc3(255, 255, 255) : ccc3(127, 127, 127);
	if (auto spr = cmi->getChildByType<ButtonSprite>(0)) {
		if (spr->m_subBGSprite) {
			spr->m_subBGSprite->setColor(color); // button bg sprite
		} else {
			if (CCScale9Sprite *cc = spr->getChildByType<CCScale9Sprite *>(0)) {
				cc->setColor(color);
			}
		}
		for (int i = 0;; i++) {
			if (auto gameObj = spr->getChildByType<GameObject>(i)) {
				setColorToGameObjectNew(gameObj, isBright);
			} else
				break;
		}
	}
}

static bool CreativeModeEffect() {
	if (Loader::get()->isModInstalled("alphalaneous.creative_mode")) {
		auto creativeMode = Loader::get()->getInstalledMod("alphalaneous.creative_mode");
		if (creativeMode->isEnabled() && creativeMode->hasSetting("enable-new-tab-ui")) {
			return creativeMode->getSettingValue<bool>("enable-new-tab-ui");
		}
	}
	return false;
}

CreateMenuItem *getCreateBtn(int param_1, int param_2) {
	/*
	 * WARNING: Removing unreachable blocks for optimization.
	 * Globals starting with '_' may overlap smaller symbols at the same address.
	 *
	 * EditorUI::getCreateBtn(int, int)
	 */
	/*            CCSprite *sprite = nullptr;
	    ObjectToolbox *toolbox = nullptr;
	    CCNode *buttonNode = nullptr;
	    CreateMenuItem *buttonObject = nullptr;
	    std::string buttonImage;
	    float scaleFactor = 1.0f;

	    switch (param_2) {
	        case 2: buttonImage = "GJ_button_02.png"; break;
	        case 3: buttonImage = "GJ_button_03.png"; break;
	        case 4: buttonImage = "GJ_button_04.png"; break;
	        case 5: buttonImage = "GJ_button_05.png"; break;
	        case 6: buttonImage = "GJ_button_06.png"; break;
	        default: buttonImage = "GJ_button_01.png";
	    }

	    if (param_1 == 0x64f || param_1 == 0x392) {
	        auto textureCache = cocos2d::CCTextureCache::sharedTextureCache();
	        auto fontTexture = textureCache->addImage("bigFont.fnt", false);
	        sprite = TextGameObject::create(fontTexture);
	    } else {
	        sprite = GameObject::createWithKey(param_1);
	    }

	    toolbox = ObjectToolbox::sharedState();
	    auto frameData = toolbox->intKeyToFrame(param_1);

	    // hacky fix for certain objects with colors sprites breaking
	    if (param_1 < 4401) {
	        if (GameObject* godspr = typeinfo_cast<GameObject*>(sprite) ) {
	            godspr->addColorSprite(frameData);
	            godspr->setupCustomSprites(frameData);
	            //log::debug("weird value used for {}({}) is {}",param_1,frameData,godspr->m_unk280);
	        }
	    }
	    switch (param_1) {
	        case 0x630:  // Case for iVar2 == 0x630 (1584) aka rob's color check
	        case 918:
	        case 1372:
	        case 1328:
	        case 1582:
	        case 94:
	        case 919:
	        case 0x7dc:
	            if (GameObject* godspr = typeinfo_cast<GameObject*>(sprite) ) {
	                godspr->setObjectColor({ 0, 0, 0 });
	            } else {
	                sprite->setColor({ 0, 0, 0 });
	            } // black
	            break;
	        case 0x64f: // text fix
	            typeinfo_cast<TextGameObject*>(sprite)->updateTextObject("0", 1);
	            break;
	        case 0x392:
	            typeinfo_cast<TextGameObject*>(sprite)->updateTextObject("A", 1);
	            break;
	        case 0x419: // color fix
	              sprite->getChildByType<CCSprite*>(0)->setColor({255, 63, 63});
	              break;
	        default:
	            break;
	    }


	    buttonNode = ButtonSprite::create(
	        sprite, 0x20, 0, 32.0f, scaleFactor, true, buttonImage.c_str(), true
	    );
	    if(sprite->getContentSize().height > (buttonNode->getContentSize().height - 5) && sprite->getContentSize().height > sprite->getContentSize().width){
	        sprite->setScale((buttonNode->getContentSize().height - 5) /sprite->getContentSize().height);
	    }
	    else if (sprite->getContentSize().width > (buttonNode->getContentSize().height - 5) ){
	        sprite->setScale((buttonNode->getContentSize().height - 5) /sprite->getContentSize().width);
	    }
	    buttonObject = CreateMenuItem::create(buttonNode, nullptr, buttonNode, nullptr);
	    buttonObject->m_objectID = param_1; // make sure it cool
	    buttonObject->setID(fmt::format("viper.object_pinning/popup-object-{}",param_1));
	    return buttonObject;*/
	EditorUI *Ui = EditorUI::get();
	auto Create = Ui->getCreateBtn(param_1, param_2);
	Create->setID(fmt::format("viper.object_pinning/popup-object-{}", param_1));
	return Create;
};
}; // namespace ReversedGDClass