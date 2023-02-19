#include "Edit.hpp"
#include <Geode/utils/general.hpp>
#include <Geode/loader/Log.hpp>
#include <Geode/utils/cocos.hpp>
#include <Geode/utils/string.hpp>
#include <Geode/ui/EnterLayerEvent.hpp>
#include <Geode/utils/file.hpp>

#define PARSE_ATTR(name, type) \
    if (auto attr = node.attribute(#name)) {\
        m_##name = attr.as_##type();\
    }

Result<std::shared_ptr<NodeEdit>> NodeEdit::from(
    pugi::xml_node const& node,
    NodeEdit* parent
) {
    std::shared_ptr<NodeEdit> edit;
    switch (hash(node.name())) {
        case hash("node"): {
            edit = std::make_shared<NodeEdit>();
        } break;

        case hash("label"): {
            edit = std::make_shared<LabelEdit>();
        } break;

        case hash("sprite"): {
            edit = std::make_shared<SpriteEdit>();
        } break;

        case hash("scene"): {
            edit = std::make_shared<SceneEdit>();
        } break;

        default: {
            return Err("Unknown node type \"" + std::string(node.name()) + "\"");
        } break;
    }
    edit->m_parent = parent;
    auto res = edit->parse(node);
    if (!res) {
        return Err(res.unwrapErr());
    }
    return Ok(edit);
}

Result<> NodeEdit::parse(pugi::xml_node const& node) {
    PARSE_ATTR(id, string);
    PARSE_ATTR(x, float);
    PARSE_ATTR(y, float);
    PARSE_ATTR(width, float);
    PARSE_ATTR(height, float);
    PARSE_ATTR(layout, string);
    PARSE_ATTR(create, bool);

    for (auto child : node.children()) {
        if (child.name() && strlen(child.name())) {
            auto c = NodeEdit::from(child, this);
            if (!c) {
                return Err(c.unwrapErr());
            }
            m_children.push_back(c.unwrap());
        }
    }

    return Ok();
}

Layout* NodeEdit::createLayout(std::string const& expr) {
    auto name = expr;
    std::vector<std::string> params;
    if (string::contains(expr, "(")) {
        name = expr.substr(0, expr.find("("));
        auto p = expr.substr(expr.find("(") + 1, expr.find(")") - expr.find("(") - 1);
        params = string::split(p, ",");
    }
    switch (hash(name.c_str())) {
        case hash("column"): case hash("vertical"): {
            float gap = 5.f;
            try { gap = std::stof(params[0]); } catch(...) {}
            return ColumnLayout::create()->setGap(gap);
        } break;

        case hash("row"): case hash("horizontal"): {
            float gap = 5.f;
            try { gap = std::stof(params[0]); } catch(...) {}
            return RowLayout::create()->setGap(gap);
        } break;

        default: return nullptr;
    }
}

void NodeEdit::apply(CCNode* node) const {
    if (m_id) node->setID(m_id.value());

    if (m_x) node->setPositionX(m_x.value());
    if (m_y) node->setPositionY(m_y.value());

    if (m_width && m_height) {
        node->setContentSize({ m_width.value(), m_height.value() });
    }
    else if (m_width) node->setContentSize({
        m_width.value(),
        node->getContentSize().height
    });
    else if (m_height) node->setContentSize({
        node->getContentSize().width,
        m_height.value()
    });

    for (auto& child : m_children) {
        if (child->m_id && !m_create) {
            if (auto n = node->getChildByID(child->m_id.value())) {
                child->apply(n);
            }
        } else {
            if (auto c = child->createNode()) {
                node->addChild(c);
                child->apply(c);
            }
        }
    }
    
    if (m_layout) {
        if (auto layout = createLayout(m_layout.value())) {
            node->setLayout(layout);
        } else {
            node->updateLayout();
        }
    } else {
        node->updateLayout();
    }
}

CCNode* NodeEdit::createNode() const {
    return CCNode::create();
}

// LabelEdit

Result<> LabelEdit::parse(pugi::xml_node const& node) {
    auto res = NodeEdit::parse(node);
    if (!res) return Err(res.unwrapErr());

    PARSE_ATTR(text, string);
    PARSE_ATTR(font, string);

    if (auto val = node.text()) {
        m_text = val.as_string();
    }

    return Ok();
}

void LabelEdit::apply(CCNode* node) const {
    NodeEdit::apply(node);
    if (auto label = dynamic_cast<CCLabelBMFont*>(node)) {
        if (m_font) {
            label->setFntFile(m_font.value().c_str());
        }
        if (m_text) {
            label->setString(m_text.value().c_str());
        }
    }
}

CCNode* LabelEdit::createNode() const {
    return CCLabelBMFont::create(
        m_text.value_or("").c_str(),
        m_font.value_or("bigFont.fnt").c_str()
    );
}

// SpriteEdit

Result<> SpriteEdit::parse(pugi::xml_node const& node) {
    auto res = NodeEdit::parse(node);
    if (!res) return Err(res.unwrapErr());

    PARSE_ATTR(src, string);
    PARSE_ATTR(frame, string);

    return Ok();
}

void SpriteEdit::apply(CCNode* node) const {
    NodeEdit::apply(node);
    if (auto spr = dynamic_cast<CCSprite*>(node)) {
        if (m_src) {
            if (!m_src.value().size()) {
                spr->setTexture(nullptr);
                spr->setTextureRect(CCRectZero);
            }
            else if (auto texture = CCTextureCache::get()->addImage(
                m_src.value().c_str(), false
            )) {
                spr->setTexture(texture);
                spr->setTextureRect({
                    0, 0,
                    texture->getContentSize().width,
                    texture->getContentSize().height
                });
            }
        }
        if (m_frame) {
            if (auto frame = CCSpriteFrameCache::get()->spriteFrameByName(
                m_frame.value().c_str()
            )) {
                spr->setDisplayFrame(frame);
            }
        }
    }
}

CCNode* SpriteEdit::createNode() const {
    if (m_frame) {
        return CCSprite::createWithSpriteFrameName(m_frame.value().c_str());
    }
    if (m_src) {
        return CCSprite::create(m_src.value().c_str());
    }
    return CCSprite::create();
}

// SceneEdit

Result<> SceneEdit::parse(pugi::xml_node const& node) {
    auto res = NodeEdit::parse(node);
    if (!res) return Err(res.unwrapErr());

    m_listener = std::make_shared<EventListener<AEnterLayerFilter>>(
        [this](AEnterLayerEvent* event) {
            this->apply(event->layer);
        },
        AEnterLayerFilter(m_id)
    );
    
    return Ok();
}

CCNode* SceneEdit::createNode() const {
    return CCScene::create();
}

// EditCollection

Result<> EditCollection::addFrom(ghc::filesystem::path const& path) {
    auto data = file::readString(path);
    if (!data) {
        return Err("Unable to read file: " + data.error());
    }
    pugi::xml_document doc;
    auto res = doc.load(data.value().c_str());
    if (res.status != pugi::status_ok) {
        return Err("Unable to parse XML: " + std::string(res.description()));
    }

    // start from "textureldr" node, or if that's not found just assume the 
    // document's valid edit XML and start from root
    auto root = doc.root().child("textureldr");
    if (!root) {
        root = doc.root();
    }

    // only scenes allowed at top level
    for (auto scene : root.children("scene")) {
        auto editRes = NodeEdit::from(scene);
        if (!editRes) {
            return Err(editRes.unwrapErr());
        }
        auto edit = editRes.unwrap();
        if (!edit->m_id) {
            return Err("Scene is missing ID");
        }
        // merge existing handlers to avoid adding too many
        if (m_edits.count(edit->m_id.value())) {
            ranges::push(
                m_edits.at(edit->m_id.value())->m_children,
                edit->m_children
            );
        } else {
            m_edits.insert({ edit->m_id.value(), edit });
        }
    }
    return Ok();
}

void EditCollection::clear() {
    m_edits.clear();
}
