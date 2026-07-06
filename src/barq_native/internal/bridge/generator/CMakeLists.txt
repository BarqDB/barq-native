set(BRIDGE_TYPE_REQUIRED_HEADERS
    barq.hpp
    barq/object-store/dictionary.hpp
    barq/object-store/list.hpp
    barq/object-store/results.hpp
    barq/object-store/shared_barq.hpp
    barq/object-store/thread_safe_reference.hpp
    barq/sync/config.hpp
    barq/sync/subscriptions.hpp
)

set(BRIDGE_TYPES
    OwnedBinaryData barq::OwnedBinaryData
    Barq_Config barq::Barq::Config
    ColKey barq::ColKey
    Decimal128 barq::Decimal128
    Dictionary barq::object_store::Dictionary
    CoreDictionary barq::Dictionary
    List barq::List
    LnkLst barq::LnkLst
    Mixed barq::Mixed
    ObjKey barq::ObjKey
    ObjLink barq::ObjLink
    Obj barq::Obj
    ObjectId barq::ObjectId
    ObjectSchema barq::ObjectSchema
    Object barq::Object
    IndexSet barq::IndexSet
    CollectionChangeSet barq::CollectionChangeSet
    IndexSet_IndexIterator barq::IndexSet::IndexIterator
    IndexSet_IndexIteratableAdaptor barq::IndexSet::IndexIteratableAdaptor
    NotificationToken barq::NotificationToken
    Property barq::Property
    Query barq::Query
    Results barq::Results
    Schema barq::Schema
    Set barq::object_store::Set
    SyncError barq::SyncError
    TableRef barq::TableRef
    TableView barq::TableView
    ThreadSafeReference barq::ThreadSafeReference
    Timestamp barq::Timestamp
    UUID barq::UUID
    Status barq::Status
    ErrorCategory barq::ErrorCategory
    SyncSubscriptionSet barq::sync::SubscriptionSet
    MutableSyncSubscriptionSet barq::sync::MutableSubscriptionSet
)

set(TYPE_INFO_QUERY_DECLS "")

while(BRIDGE_TYPES)
    list(POP_FRONT BRIDGE_TYPES _name _expression)
    string(REGEX MATCHALL "." _name_char_matches "${_name}")
    list(TRANSFORM _name_char_matches PREPEND ')
    list(TRANSFORM _name_char_matches APPEND ')
    list(JOIN _name_char_matches ", " _name_chars)
    set(TYPE_INFO_QUERY_DECLS "${TYPE_INFO_QUERY_DECLS}    BARQ_TYPE_INFO(${_name}, ${_expression}, ${_name_chars});\n")
endwhile()

foreach(h ${BRIDGE_TYPE_REQUIRED_HEADERS})
string(APPEND BRIDGE_TYPE_HEADER_DECLS "#include \"${h}\"\n")
endforeach()

configure_file(bridge_type_info_generator.cpp.in bridge_type_info_generator.cpp)
add_library(BridgeTypeInfoGenerator STATIC ${CMAKE_CURRENT_BINARY_DIR}/bridge_type_info_generator.cpp)

if(MSVC)
    target_compile_options(BridgeTypeInfoGenerator PRIVATE /O0)
else()
    target_compile_options(BridgeTypeInfoGenerator PRIVATE -O0)
endif()

target_link_libraries(BridgeTypeInfoGenerator Barq::Storage)
