#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace ReversedGDClass {

    CreateMenuItem* getCreateBtn(int param_1, int param_2) {
        /*
        * WARNING: Removing unreachable blocks for optimization.
        * Globals starting with '_' may overlap smaller symbols at the same address.
        *
        * EditorUI::getCreateBtn(int, int)
        */

            CCSprite *sprite = nullptr;
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
            return buttonObject;
    };
};