
enum EntityStateFlag
{
    ACTIVE = 1 << 0,
    TO_BE_DESTROYED = 1 << 1,
};

struct Entity;

template<typename T>
struct EntityRef
{

    u32 id;
    u32 generation;

    T *operator*()
    {
        if (generation > state->entity_generations[id]) {
            return NULL;
        }

        return (T*) (state->entity_slots + id);
    }

    T *operator->()
    {
        if (generation > state->entity_generations[id]) {
            return NULL;
        }

        return (T*) (state->entity_slots + id);
    }

};

template<typename T>
T* AllocateEntity();

template<typename T>
void DeleteEntity(T *entity);

template<typename T>
EntityRef<T> MakeRef(Entity *target);


struct Entity
{
    u32 id;
    u32 generation;
    u32 flags;

    Vector3 local_position;
    Vector3 local_rotation;
    Vector3 local_scale = {1, 1, 1};

    EntityRef<Entity> parent;
    EntityRef<Entity> child;
    EntityRef<Entity> next;

    Vector3 GetWorldPosition()
    {
        if (!*parent) {
            return local_position;
        }
        return Vector3Transform(local_position, parent->GetWorldTransform());
    }

    Matrix GetLocalTransform() const
    {

        return MatrixMultiply(
            MatrixTranslate(local_position.x, local_position.y, local_position.z),
            MatrixMultiply(
                MatrixRotateXYZ(local_rotation),
                MatrixScale(local_scale.x, local_scale.y, local_scale.z)
            )
        );
    }

    Matrix GetWorldTransform()
    {
        if (!*parent) {
            return GetLocalTransform();
        }
        return MatrixMultiply(parent->GetWorldTransform(), GetLocalTransform());
    }


    virtual void OnCreate()
    {

    };

    virtual void OnEnable()
    {
        flags = flags | EntityStateFlag::ACTIVE;
        Entity *next_target = *child;
        while (next_target) {
            next_target->OnEnable();
            next_target = *next_target->next;
        }
    }

    virtual void Update()
    {
        Entity *next_target = *child;
        while (next_target) {
            next_target->Update();
            next_target = *next_target->next;
        }
    }

    virtual void Render()
    {
        rlPushMatrix();
        rlTranslatef(local_position.x, local_position.y, local_position.z);
        rlRotatef(local_rotation.x,1, 0,0);
        rlRotatef(local_rotation.y,0, 1,0);
        rlRotatef(local_rotation.z,0, 0,1);
        rlScalef(local_scale.x, local_scale.y, local_scale.z);
        Entity *next_target = *child;
        while (next_target) {
            next_target->Render();
            next_target = *next_target->next;
        }

        rlPopMatrix();
    }

    virtual void OnDisable()
    {
        flags = flags & ~EntityStateFlag::ACTIVE;
        Entity *next_target = *child;
        while (next_target) {
            next_target->OnDisable();
            next_target = *next_target->next;
        }
    }

    virtual void OnDestroy()
    {
        if (*parent && parent->flags & EntityStateFlag::ACTIVE) {
            SetParent(nullptr);
        }

        flags = flags | EntityStateFlag::TO_BE_DESTROYED;
        Entity *next_target = *child;
        while (next_target != nullptr) {
            next_target->OnDestroy();
            next_target = *next_target->next;
        }
    }

    void PushChild(Entity *new_child)
    {
        if (*new_child->parent) {
            new_child->SetParent(nullptr);
        }
        new_child->next = child;

        child = MakeRef<Entity>(new_child);
        child->parent = MakeRef<Entity>(this);

        // Activate new child if required and self active
        if (new_child->flags & EntityStateFlag::ACTIVE) return;
        if (flags & ~EntityStateFlag::ACTIVE) return;
        new_child->OnEnable();
    }

    void RemoveChild(Entity *child_ref)
    {
        Entity *previous = nullptr;
        Entity *current = *child;

        while (current) {

            if (current->id == child_ref->id) {

                if (previous) {
                    previous->next = current->next;
                    current->parent = MakeRef<Entity>(nullptr);
                } else {
                    child = current->next;
                    current->parent = MakeRef<Entity>(nullptr);
                }

                if (current->flags & EntityStateFlag::ACTIVE) current->OnDisable();

                return;
            }

            previous = current;
            current = *child->next;
        }

    }

    void SetParent(Entity *new_parent)
    {
        if (*parent) {
            RemoveChild(this);
        }

        if(new_parent) {
            new_parent->PushChild(this);
        } else {
            parent = {};
        }
    }

    Entity *PopChild()
    {
        Entity *old_child = *child;
        old_child->SetParent(nullptr);
        return old_child;
    }
};

template<typename T>
EntityRef<T> MakeRef(Entity *target)
{
    return { target->id, target->generation };
};

template<typename T>
T* AllocateEntity()
{
    u32 id = state->free_entities[state->free_entity_count];
    state->free_entity_count--;

    T *entity = new ((void*) (state->entity_slots + id)) T;

    entity->id = id;
    entity->generation = state->entity_generations[id];

    return entity;
}

template<typename T>
void DeleteEntity(T *entity)
{

}
