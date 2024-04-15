struct CardScene: Entity
{

    void Render() override
    {
    }

    void RenderGUI() override
    {
        Entity::Render();
        Entity::RenderGUI();
    }

};
