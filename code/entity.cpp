
enum EntityStateFlag
{
    ACTIVE = 1 << 0,
    TO_BE_DESTROYED = 1 << 1,
};

struct Entity;

template<typename T>
struct EntityRef
{
    T *operator*()
    {
        return ref;
    }

    T *operator->()
    {
        return ref;
    }

    T *ref;
};

u32 counter = 0;

template<typename T>
T *allocate_entity()
{
    T *entity = new T();
    entity->id = ++counter;
    return entity;
}

template<typename T>
EntityRef<T> make_ref(Entity *target)
{
    return {
            (T *) target
    };
};

struct Entity
{

    Entity() = default;

    Vector3 local_position{};
    Vector3 local_rotation{};
    Vector3 local_scale{1,1,1};

    u32 id = 0;

    u32 flags = 0;

    EntityRef<Entity> parent{};
    EntityRef<Entity> child{};
    EntityRef<Entity> next{};

    Vector3 GetWorldPosition() {
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

        child = make_ref<Entity>(new_child);
        child->parent = make_ref<Entity>(this);

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
                    current->parent = make_ref<Entity>(nullptr);
                } else {
                    child = current->next;
                    current->parent = make_ref<Entity>(nullptr);
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

        parent = make_ref<Entity>(new_parent);
    }

    Entity *PopChild()
    {
        Entity *old_child = *child;
        old_child->SetParent(nullptr);
        return old_child;
    }
};
