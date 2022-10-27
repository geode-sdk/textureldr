#pragma once

#include <string>
#include <optional>
#include <vector>
#include <Geode/utils/Result.hpp>
#include <fs/filesystem.hpp>
#include <cocos2d.h>

USE_GEODE_NAMESPACE();

struct NodeEdit : public std::enable_shared_from_this<NodeEdit> {
    NodeEdit* m_parent = nullptr;
    std::optional<std::string> m_id;
    bool m_create = false;
    std::optional<float> m_x;
    std::optional<float> m_y;
    std::optional<float> m_width;
    std::optional<float> m_height;
    std::optional<std::string> m_layout;
    std::vector<std::shared_ptr<NodeEdit>> m_children;

    virtual ~NodeEdit() = default;

    static NewResult<std::shared_ptr<NodeEdit>> from(
        pugi::xml_node const& node,
        NodeEdit* parent = nullptr
    );

    static Layout* createLayout(std::string const& name);

    virtual NewResult<> parse(pugi::xml_node const& node);
    virtual void apply(CCNode* node) const;
    virtual CCNode* createNode() const;
};

struct SpriteEdit : public NodeEdit {
    std::optional<std::string> m_src;
    std::optional<std::string> m_frame;

    NewResult<> parse(pugi::xml_node const& node) override;
    void apply(CCNode* node) const override;
    CCNode* createNode() const override;
};

struct LabelEdit : public NodeEdit {
    std::optional<std::string> m_text;
    std::optional<std::string> m_font;

    NewResult<> parse(pugi::xml_node const& node) override;
    void apply(CCNode* node) const override;
    CCNode* createNode() const override;
};

struct SceneEdit : public NodeEdit {
    std::shared_ptr<AEnterLayerEventHandler> m_handler;

    NewResult<> parse(pugi::xml_node const& node) override;
    CCNode* createNode() const override;
};

class EditCollection final {
private:
    std::unordered_map<std::string, std::shared_ptr<NodeEdit>> m_edits;

public:
    NewResult<> addFrom(ghc::filesystem::path const& path);
    void clear();
};
