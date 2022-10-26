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

NewResult<std::shared_ptr<NodeEdit>> NodeEdit::from(
    pugi::xml_node const& node,
    std::shared_ptr<NodeEdit> parent
) {
    std::shared_ptr<NodeEdit> edit;
    switch (hash(node.name())) {
        case hash("node"): {
            edit = std::make_shared<NodeEdit>();
        } break;

        case hash("scene"): {
            edit = std::make_shared<SceneEdit>();
        } break;

        default: {
            return NewErr("Unknown node type \"" + std::string(node.name()) + "\"");
        } break;
    }
    edit->m_parent = parent;
    auto res = edit->parse(node);
    if (!res) {
        return NewErr(res.unwrapErr());
    }
    return NewOk(edit);
}

NewResult<> NodeEdit::parse(pugi::xml_node const& node) {
    PARSE_ATTR(id, string);
    PARSE_ATTR(x, float);
    PARSE_ATTR(y, float);
    PARSE_ATTR(width, float);
    PARSE_ATTR(height, float);
    PARSE_ATTR(layout, string);

    for (auto child : node.children()) {
        auto c = NodeEdit::from(child, shared_from_this());
        if (!c) {
            return NewErr(c.unwrapErr());
        }
        m_children.push_back(c.unwrap());
    }

    return NewOk();
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
            std::optional<float> align = std::nullopt;

            try { gap = std::stof(params[0]); } catch(...) {}
            try { align = std::stof(params[1]); } catch(...) {}

            return ColumnLayout::create(gap, align);
        } break;

        case hash("row"): case hash("horizontal"): {
            float gap = 5.f;
            std::optional<float> align = std::nullopt;

            try { gap = std::stof(params[0]); } catch(...) {}
            try { align = std::stof(params[1]); } catch(...) {}

            return RowLayout::create(gap, align);
        } break;

        default: return nullptr;
    }
}

void NodeEdit::apply(CCNode* node) const {
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

    if (m_layout) {
        if (auto layout = createLayout(m_layout.value())) {
            node->setLayout(layout);
        }
    }

    for (auto& child : m_children) {
        if (auto n = node->getChildByID(child->m_id)) {
            child->apply(n);
        }
    }
}

NewResult<> SceneEdit::parse(pugi::xml_node const& node) {
    auto res = NodeEdit::parse(node);
    if (!res) return NewErr(res.unwrapErr());

    m_handler = std::make_shared<AEnterLayerEventHandler>(
        m_id, [this](AEnterLayerEvent* event) {
            this->apply(event->getLayer());
        }
    );

    return NewOk();
}

NewResult<> EditCollection::addFrom(ghc::filesystem::path const& path) {
    auto data = file::readString(path);
    if (!data) {
        return NewErr("Unable to read file: " + data.error());
    }
    pugi::xml_document doc;
    auto res = doc.load(data.value().c_str());
    if (res.status != pugi::status_ok) {
        return NewErr("Unable to parse XML: " + std::string(res.description()));
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
            return NewErr(editRes.unwrapErr());
        }
        auto edit = editRes.unwrap();
        // merge existing handlers to avoid adding too many
        if (m_edits.count(edit->m_id)) {
            ranges::push(
                m_edits.at(edit->m_id)->m_children,
                edit->m_children
            );
        } else {
            m_edits.insert({ edit->m_id, edit });
        }
    }
    return NewOk();
}

void EditCollection::clear() {
    m_edits.clear();
}
