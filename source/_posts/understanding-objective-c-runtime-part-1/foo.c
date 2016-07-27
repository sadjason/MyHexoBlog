struct objc_class {
    Class isa  OBJC_ISA_AVAILABILITY;

#if !__OBJC2__
    Class super_class                                        OBJC2_UNAVAILABLE;
    const char *name                                         OBJC2_UNAVAILABLE;
    long version                                             OBJC2_UNAVAILABLE;
    long info                                                OBJC2_UNAVAILABLE;
    long instance_size                                       OBJC2_UNAVAILABLE;
    struct objc_ivar_list *ivars                             OBJC2_UNAVAILABLE;
    struct objc_method_list **methodLists                    OBJC2_UNAVAILABLE;
    struct objc_cache *cache                                 OBJC2_UNAVAILABLE;
    struct objc_protocol_list *protocols                     OBJC2_UNAVAILABLE;
#endif

}

struct objc_object {
    Class isa  OBJC_ISA_AVAILABILITY;
};

typedef struct objc_class *Class;

typedef struct objc_object *id;

typedef struct objc_selector *SEL;
// 没找到objc_selector的定义

/// A pointer to the function of a method implementation. 
#if !OBJC_OLD_DISPATCH_PROTOTYPES
typedef void (*IMP)(void /* id, SEL, ... */ ); 
#else
typedef id (*IMP)(id, SEL, ...); 
#endif

/// An opaque type that represents a method in a class definition.
typedef struct objc_method *Method;
// 没有找到objc_method的定义

/// An opaque type that represents an instance variable.
typedef struct objc_ivar *Ivar;
// 没有找到objc_ivar的定义

/// An opaque type that represents a category.
typedef struct objc_category *Category;
// 没有找到objc_category的定义

/// An opaque type that represents an Objective-C declared property.
typedef struct objc_property *objc_property_t;
// 没有找到objc_property的定义

@interface NSObject <NSObject> {
    Class isa  OBJC_ISA_AVAILABILITY;
}