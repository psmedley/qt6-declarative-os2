/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
#ifndef QQMLPROPERTYCACHECREATOR_P_H
#define QQMLPROPERTYCACHECREATOR_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <private/qqmlvaluetype_p.h>
#include <private/qqmlengine_p.h>
#include <private/qqmlmetaobject_p.h>
#include <private/qqmlpropertyresolver_p.h>
#include <private/qqmltypedata_p.h>
#include <private/inlinecomponentutils_p.h>
#include <private/qqmlsourcecoordinate_p.h>

#include <QScopedValueRollback>
#include <vector>

QT_BEGIN_NAMESPACE

inline QQmlError qQmlCompileError(const QV4::CompiledData::Location &location,
                                                  const QString &description)
{
    QQmlError error;
    error.setLine(qmlConvertSourceCoordinate<quint32, int>(location.line()));
    error.setColumn(qmlConvertSourceCoordinate<quint32, int>(location.column()));
    error.setDescription(description);
    return error;
}

struct QQmlBindingInstantiationContext {
    QQmlBindingInstantiationContext() {}
    QQmlBindingInstantiationContext(int referencingObjectIndex,
                                    const QV4::CompiledData::Binding *instantiatingBinding,
                                    const QString &instantiatingPropertyName,
                                    QQmlPropertyCache *referencingObjectPropertyCache);

    bool resolveInstantiatingProperty();
    QQmlRefPointer<QQmlPropertyCache> instantiatingPropertyCache(QQmlEnginePrivate *enginePrivate) const;

    int referencingObjectIndex = -1;
    const QV4::CompiledData::Binding *instantiatingBinding = nullptr;
    QString instantiatingPropertyName;
    QQmlRefPointer<QQmlPropertyCache> referencingObjectPropertyCache;
    QQmlPropertyData *instantiatingProperty = nullptr;
};

struct QQmlPendingGroupPropertyBindings : public QVector<QQmlBindingInstantiationContext>
{
    void resolveMissingPropertyCaches(QQmlEnginePrivate *enginePrivate, QQmlPropertyCacheVector *propertyCaches) const;
};

struct QQmlPropertyCacheCreatorBase
{
    Q_DECLARE_TR_FUNCTIONS(QQmlPropertyCacheCreatorBase)
public:
    static QAtomicInt classIndexCounter;

    static QMetaType metaTypeForPropertyType(QV4::CompiledData::BuiltinType type);

    static QByteArray createClassNameTypeByUrl(const QUrl &url);

    static QByteArray createClassNameForInlineComponent(const QUrl &baseUrl, int icId);

    struct IncrementalResult {
        // valid if and only if an error occurred
        QQmlError error;
        // true if there was no error and there are still components left to process
        bool canResume = false;
        // the object index of the last processed (inline) component root.
        int processedRoot = 0;
    };
};

template <typename ObjectContainer>
class QQmlPropertyCacheCreator : public QQmlPropertyCacheCreatorBase
{
public:
    using CompiledObject = typename ObjectContainer::CompiledObject;
    using InlineComponent = typename std::remove_reference<decltype (*(std::declval<CompiledObject>().inlineComponentsBegin()))>::type;

    QQmlPropertyCacheCreator(QQmlPropertyCacheVector *propertyCaches,
                             QQmlPendingGroupPropertyBindings *pendingGroupPropertyBindings,
                             QQmlEnginePrivate *enginePrivate,
                             const ObjectContainer *objectContainer, const QQmlImports *imports,
                             const QByteArray &typeClassName);


    /*!
        \internal
        Creates the property cache for the CompiledObjects of objectContainer,
        one (inline) root component at a time.

        \note Later compiler passes might modify those property caches. Therefore,
        the actual metaobjects are not created yet.
     */
    IncrementalResult buildMetaObjectsIncrementally();

    /*!
        \internal
        Returns a valid error if the inline components of the objectContainer
        form a cycle. Otherwise an invalid error is returned
     */
    QQmlError verifyNoICCycle();

    /*!
        \internal
        Fills the property caches for the CompiledObjects by
        calling buildMetaObjectsIncrementally until it can no
        longer resume.
     */
    QQmlError buildMetaObjects();

    enum class VMEMetaObjectIsRequired {
        Maybe,
        Always
    };
protected:
    QQmlError buildMetaObjectRecursively(int objectIndex, const QQmlBindingInstantiationContext &context, VMEMetaObjectIsRequired isVMERequired);
    QQmlRefPointer<QQmlPropertyCache> propertyCacheForObject(const CompiledObject *obj, const QQmlBindingInstantiationContext &context, QQmlError *error) const;
    QQmlError createMetaObject(int objectIndex, const CompiledObject *obj, const QQmlRefPointer<QQmlPropertyCache> &baseTypeCache);

    QMetaType metaTypeForParameter(const QV4::CompiledData::ParameterType &param, QString *customTypeName = nullptr);

    QString stringAt(int index) const { return objectContainer->stringAt(index); }

    QQmlEnginePrivate * const enginePrivate;
    const ObjectContainer * const objectContainer;
    const QQmlImports * const imports;
    QQmlPropertyCacheVector *propertyCaches;
    QQmlPendingGroupPropertyBindings *pendingGroupPropertyBindings;
    QByteArray typeClassName; // not const as we temporarily chang it for inline components
    unsigned int currentRoot; // set to objectID of inline component root when handling inline components

    QQmlBindingInstantiationContext m_context;
    std::vector<InlineComponent> allICs;
    std::vector<icutils::Node> nodesSorted;
    std::vector<icutils::Node>::reverse_iterator nodeIt = nodesSorted.rbegin();
    bool hasCycle = false;
};

template <typename ObjectContainer>
inline QQmlPropertyCacheCreator<ObjectContainer>::QQmlPropertyCacheCreator(QQmlPropertyCacheVector *propertyCaches,
                                                                           QQmlPendingGroupPropertyBindings *pendingGroupPropertyBindings,
                                                                           QQmlEnginePrivate *enginePrivate,
                                                                           const ObjectContainer *objectContainer, const QQmlImports *imports,
                                                                           const QByteArray &typeClassName)
    : enginePrivate(enginePrivate)
    , objectContainer(objectContainer)
    , imports(imports)
    , propertyCaches(propertyCaches)
    , pendingGroupPropertyBindings(pendingGroupPropertyBindings)
    , typeClassName(typeClassName)
    , currentRoot(-1)
{
    propertyCaches->resize(objectContainer->objectCount());

    using namespace icutils;

    // get a list of all inline components

    for (int i=0; i != objectContainer->objectCount(); ++i) {
        const CompiledObject *obj = objectContainer->objectAt(i);
        for (auto it = obj->inlineComponentsBegin(); it != obj->inlineComponentsEnd(); ++it) {
            allICs.push_back(*it);
        }
    }

    // create a graph on inline components referencing inline components
    std::vector<Node> nodes;
    nodes.resize(allICs.size());
    std::iota(nodes.begin(), nodes.end(), 0);
    AdjacencyList adjacencyList;
    adjacencyList.resize(nodes.size());
    fillAdjacencyListForInlineComponents(objectContainer, adjacencyList, nodes, allICs);

    nodesSorted = topoSort(nodes, adjacencyList, hasCycle);
    nodeIt = nodesSorted.rbegin();
}

template <typename ObjectContainer>
inline QQmlError QQmlPropertyCacheCreator<ObjectContainer>::verifyNoICCycle()
{
    if (hasCycle) {
        QQmlError diag;
        diag.setDescription(QLatin1String("Inline components form a cycle!"));
        return diag;
    }
    return {};
}

template <typename ObjectContainer>
inline  QQmlPropertyCacheCreatorBase::IncrementalResult
QQmlPropertyCacheCreator<ObjectContainer>::buildMetaObjectsIncrementally()
{
    // needs to be checked with verifyNoICCycle before this function is called
    Q_ASSERT(!hasCycle);

    // create meta objects for inline components before compiling actual root component
    if (nodeIt != nodesSorted.rend()) {
        const auto &ic = allICs[nodeIt->index()];
        QV4::ResolvedTypeReference *typeRef = objectContainer->resolvedType(ic.nameIndex);
        Q_ASSERT(propertyCaches->at(ic.objectIndex) == nullptr);
        Q_ASSERT(typeRef->typePropertyCache().isNull()); // not set yet

        QByteArray icTypeName { objectContainer->stringAt(ic.nameIndex).toUtf8() };
        QScopedValueRollback<QByteArray> nameChange {typeClassName, icTypeName};
        QScopedValueRollback<unsigned int> rootChange {currentRoot, ic.objectIndex};
        ++nodeIt;
        QQmlError diag = buildMetaObjectRecursively(ic.objectIndex, m_context, VMEMetaObjectIsRequired::Always);
        if (diag.isValid()) {
            return {diag, false, 0};
        }
        typeRef->setTypePropertyCache(propertyCaches->at(ic.objectIndex));
        Q_ASSERT(!typeRef->typePropertyCache().isNull());
        return { QQmlError(), true, int(ic.objectIndex) };
    }

    auto diag = buildMetaObjectRecursively(/*root object*/0, m_context, VMEMetaObjectIsRequired::Maybe);
    return {diag, false, 0};
}

template <typename ObjectContainer>
inline  QQmlError
QQmlPropertyCacheCreator<ObjectContainer>::buildMetaObjects()
{
    QQmlError error = verifyNoICCycle();
    if (error.isValid())
        return error;
    QQmlPropertyCacheCreatorBase::IncrementalResult result;
    do {
        result = buildMetaObjectsIncrementally();
    } while (result.canResume);
    return result.error;
}

template <typename ObjectContainer>
inline QQmlError QQmlPropertyCacheCreator<ObjectContainer>::buildMetaObjectRecursively(int objectIndex, const QQmlBindingInstantiationContext &context, VMEMetaObjectIsRequired isVMERequired)
{
    auto isAddressable = [](const QUrl &url) {
        const QString fileName = url.fileName();
        return !fileName.isEmpty() && fileName.front().isUpper();
    };

    const CompiledObject *obj = objectContainer->objectAt(objectIndex);
    bool needVMEMetaObject = isVMERequired == VMEMetaObjectIsRequired::Always || obj->propertyCount() != 0 || obj->aliasCount() != 0
            || obj->signalCount() != 0 || obj->functionCount() != 0 || obj->enumCount() != 0
            || ((obj->hasFlag(QV4::CompiledData::Object::IsComponent)
                 || (objectIndex == 0 && isAddressable(objectContainer->url())))
                && !objectContainer->resolvedType(obj->inheritedTypeNameIndex)->isFullyDynamicType());

    if (!needVMEMetaObject) {
        auto binding = obj->bindingsBegin();
        auto end = obj->bindingsEnd();
        for ( ; binding != end; ++binding) {
            if (binding->type() == QV4::CompiledData::Binding::Type_Object
                    && (binding->flags() & QV4::CompiledData::Binding::IsOnAssignment)) {
                // If the on assignment is inside a group property, we need to distinguish between QObject based
                // group properties and value type group properties. For the former the base type is derived from
                // the property that references us, for the latter we only need a meta-object on the referencing object
                // because interceptors can't go to the shared value type instances.
                if (context.instantiatingProperty && QQmlMetaType::isValueType(context.instantiatingProperty->propType())) {
                    if (!propertyCaches->needsVMEMetaObject(context.referencingObjectIndex)) {
                        const CompiledObject *obj = objectContainer->objectAt(context.referencingObjectIndex);
                        auto *typeRef = objectContainer->resolvedType(obj->inheritedTypeNameIndex);
                        Q_ASSERT(typeRef);
                        QQmlRefPointer<QQmlPropertyCache> baseTypeCache = typeRef->createPropertyCache(QQmlEnginePrivate::get(enginePrivate));
                        QQmlError error = createMetaObject(context.referencingObjectIndex, obj, baseTypeCache);
                        if (error.isValid())
                            return error;
                    }
                } else {
                    // On assignments are implemented using value interceptors, which require a VME meta object.
                    needVMEMetaObject = true;
                }
                break;
            }
        }
    }

    QQmlRefPointer<QQmlPropertyCache> baseTypeCache;
    {
        QQmlError error;
        baseTypeCache = propertyCacheForObject(obj, context, &error);
        if (error.isValid())
            return error;
    }

    if (baseTypeCache) {
        if (needVMEMetaObject) {
            QQmlError error = createMetaObject(objectIndex, obj, baseTypeCache);
            if (error.isValid())
                return error;
        } else {
            propertyCaches->set(objectIndex, baseTypeCache);
        }
    }

    QQmlPropertyCache *thisCache = propertyCaches->at(objectIndex);
    auto binding = obj->bindingsBegin();
    auto end = obj->bindingsEnd();
    for ( ; binding != end; ++binding) {
        if (binding->type() >= QV4::CompiledData::Binding::Type_Object) {
            QQmlBindingInstantiationContext context(objectIndex, &(*binding), stringAt(binding->propertyNameIndex), thisCache);

            // Binding to group property where we failed to look up the type of the
            // property? Possibly a group property that is an alias that's not resolved yet.
            // Let's attempt to resolve it after we're done with the aliases and fill in the
            // propertyCaches entry then.
            if (!thisCache || !context.resolveInstantiatingProperty())
                pendingGroupPropertyBindings->append(context);

            QQmlError error = buildMetaObjectRecursively(binding->value.objectIndex, context, VMEMetaObjectIsRequired::Maybe);
            if (error.isValid())
                return error;
        }
    }

    QQmlError noError;
    return noError;
}

template <typename ObjectContainer>
inline QQmlRefPointer<QQmlPropertyCache> QQmlPropertyCacheCreator<ObjectContainer>::propertyCacheForObject(const CompiledObject *obj, const QQmlBindingInstantiationContext &context, QQmlError *error) const
{
    if (context.instantiatingProperty) {
        return context.instantiatingPropertyCache(enginePrivate);
    } else if (obj->inheritedTypeNameIndex != 0) {
        auto *typeRef = objectContainer->resolvedType(obj->inheritedTypeNameIndex);
        Q_ASSERT(typeRef);

        if (typeRef->isFullyDynamicType()) {
            if (obj->propertyCount() > 0 || obj->aliasCount() > 0) {
                *error = qQmlCompileError(obj->location, QQmlPropertyCacheCreatorBase::tr("Fully dynamic types cannot declare new properties."));
                return nullptr;
            }
            if (obj->signalCount() > 0) {
                *error = qQmlCompileError(obj->location, QQmlPropertyCacheCreatorBase::tr("Fully dynamic types cannot declare new signals."));
                return nullptr;
            }
            if (obj->functionCount() > 0) {
                *error = qQmlCompileError(obj->location, QQmlPropertyCacheCreatorBase::tr("Fully Dynamic types cannot declare new functions."));
                return nullptr;
            }
        }

        return typeRef->createPropertyCache(QQmlEnginePrivate::get(enginePrivate));
    } else if (context.instantiatingBinding && context.instantiatingBinding->isAttachedProperty()) {
        auto *typeRef = objectContainer->resolvedType(
                context.instantiatingBinding->propertyNameIndex);
        Q_ASSERT(typeRef);
        QQmlType qmltype = typeRef->type();
        if (!qmltype.isValid()) {
            imports->resolveType(stringAt(context.instantiatingBinding->propertyNameIndex),
                                 &qmltype, nullptr, nullptr, nullptr);
        }

        const QMetaObject *attachedMo = qmltype.attachedPropertiesType(enginePrivate);
        if (!attachedMo) {
            *error = qQmlCompileError(context.instantiatingBinding->location, QQmlPropertyCacheCreatorBase::tr("Non-existent attached object"));
            return nullptr;
        }
        return enginePrivate->cache(attachedMo);
    }
    return nullptr;
}

template <typename ObjectContainer>
inline QQmlError QQmlPropertyCacheCreator<ObjectContainer>::createMetaObject(int objectIndex, const CompiledObject *obj, const QQmlRefPointer<QQmlPropertyCache> &baseTypeCache)
{
    QQmlRefPointer<QQmlPropertyCache> cache;
    cache.adopt(baseTypeCache->copyAndReserve(obj->propertyCount() + obj->aliasCount(),
                                              obj->functionCount() + obj->propertyCount() + obj->aliasCount() + obj->signalCount(),
                                              obj->signalCount() + obj->propertyCount() + obj->aliasCount(), obj->enumCount()));

    propertyCaches->set(objectIndex, cache);
    propertyCaches->setNeedsVMEMetaObject(objectIndex);

    QByteArray newClassName;

    if (objectIndex == /*root object*/0 || int(currentRoot) == objectIndex) {
        newClassName = typeClassName;
    }
    if (newClassName.isEmpty()) {
        newClassName = QQmlMetaObject(baseTypeCache.data()).className();
        newClassName.append("_QML_");
        newClassName.append(QByteArray::number(classIndexCounter.fetchAndAddRelaxed(1)));
    }

    cache->_dynamicClassName = newClassName;

    QQmlPropertyResolver resolver(baseTypeCache);

    auto p = obj->propertiesBegin();
    auto pend = obj->propertiesEnd();
    for ( ; p != pend; ++p) {
        bool notInRevision = false;
        QQmlPropertyData *d = resolver.property(stringAt(p->nameIndex), &notInRevision);
        if (d && d->isFinal())
            return qQmlCompileError(p->location, QQmlPropertyCacheCreatorBase::tr("Cannot override FINAL property"));
    }

    auto a = obj->aliasesBegin();
    auto aend = obj->aliasesEnd();
    for ( ; a != aend; ++a) {
        bool notInRevision = false;
        QQmlPropertyData *d = resolver.property(stringAt(a->nameIndex()), &notInRevision);
        if (d && d->isFinal())
            return qQmlCompileError(a->location, QQmlPropertyCacheCreatorBase::tr("Cannot override FINAL property"));
    }

    int effectivePropertyIndex = cache->propertyIndexCacheStart;
    int effectiveMethodIndex = cache->methodIndexCacheStart;

    // For property change signal override detection.
    // We prepopulate a set of signal names which already exist in the object,
    // and throw an error if there is a signal/method defined as an override.
    QSet<QString> seenSignals;
    seenSignals << QStringLiteral("destroyed") << QStringLiteral("parentChanged") << QStringLiteral("objectNameChanged");
    QQmlPropertyCache *parentCache = cache.data();
    while ((parentCache = parentCache->parent())) {
        if (int pSigCount = parentCache->signalCount()) {
            int pSigOffset = parentCache->signalOffset();
            for (int i = pSigOffset; i < pSigCount; ++i) {
                QQmlPropertyData *currPSig = parentCache->signal(i);
                // XXX TODO: find a better way to get signal name from the property data :-/
                for (QQmlPropertyCache::StringCache::ConstIterator iter = parentCache->stringCache.begin();
                     iter != parentCache->stringCache.end(); ++iter) {
                    if (currPSig == (*iter).second) {
                        seenSignals.insert(iter.key());
                        break;
                    }
                }
            }
        }
    }

    // Set up notify signals for properties - first normal, then alias
    p = obj->propertiesBegin();
    pend = obj->propertiesEnd();
    for (  ; p != pend; ++p) {
        auto flags = QQmlPropertyData::defaultSignalFlags();

        QString changedSigName = stringAt(p->nameIndex) + QLatin1String("Changed");
        seenSignals.insert(changedSigName);

        cache->appendSignal(changedSigName, flags, effectiveMethodIndex++);
    }

    a = obj->aliasesBegin();
    aend = obj->aliasesEnd();
    for ( ; a != aend; ++a) {
        auto flags = QQmlPropertyData::defaultSignalFlags();

        QString changedSigName = stringAt(a->nameIndex()) + QLatin1String("Changed");
        seenSignals.insert(changedSigName);

        cache->appendSignal(changedSigName, flags, effectiveMethodIndex++);
    }

    auto e = obj->enumsBegin();
    auto eend = obj->enumsEnd();
    for ( ; e != eend; ++e) {
        const int enumValueCount = e->enumValueCount();
        QVector<QQmlEnumValue> values;
        values.reserve(enumValueCount);

        auto enumValue = e->enumValuesBegin();
        auto end = e->enumValuesEnd();
        for ( ; enumValue != end; ++enumValue)
            values.append(QQmlEnumValue(stringAt(enumValue->nameIndex), enumValue->value));

        cache->appendEnum(stringAt(e->nameIndex), values);
    }

    // Dynamic signals
    auto s = obj->signalsBegin();
    auto send = obj->signalsEnd();
    for ( ; s != send; ++s) {
        const int paramCount = s->parameterCount();

        QList<QByteArray> names;
        names.reserve(paramCount);
        QVarLengthArray<QMetaType, 10> paramTypes(paramCount);

        if (paramCount) {

            int i = 0;
            auto param = s->parametersBegin();
            auto end = s->parametersEnd();
            for ( ; param != end; ++param, ++i) {
                names.append(stringAt(param->nameIndex).toUtf8());

                QString customTypeName;
                QMetaType type = metaTypeForParameter(param->type, &customTypeName);
                if (!type.isValid())
                    return qQmlCompileError(s->location, QQmlPropertyCacheCreatorBase::tr("Invalid signal parameter type: %1").arg(customTypeName));

                paramTypes[i] = type;
            }
        }

        auto flags = QQmlPropertyData::defaultSignalFlags();
        if (paramCount)
            flags.setHasArguments(true);

        QString signalName = stringAt(s->nameIndex);
        if (seenSignals.contains(signalName))
            return qQmlCompileError(s->location, QQmlPropertyCacheCreatorBase::tr("Duplicate signal name: invalid override of property change signal or superclass signal"));
        seenSignals.insert(signalName);

        cache->appendSignal(signalName, flags, effectiveMethodIndex++,
                            paramCount?paramTypes.constData():nullptr, names);
    }


    // Dynamic slots
    auto function = objectContainer->objectFunctionsBegin(obj);
    auto fend = objectContainer->objectFunctionsEnd(obj);
    for ( ; function != fend; ++function) {
        auto flags = QQmlPropertyData::defaultSlotFlags();

        const QString slotName = stringAt(function->nameIndex);
        if (seenSignals.contains(slotName))
            return qQmlCompileError(function->location, QQmlPropertyCacheCreatorBase::tr("Duplicate method name: invalid override of property change signal or superclass signal"));
        // Note: we don't append slotName to the seenSignals list, since we don't
        // protect against overriding change signals or methods with properties.

        QList<QByteArray> parameterNames;
        QVector<QMetaType> parameterTypes;
        auto formal = function->formalsBegin();
        auto end = function->formalsEnd();
        for ( ; formal != end; ++formal) {
            flags.setHasArguments(true);
            parameterNames << stringAt(formal->nameIndex).toUtf8();
            QMetaType type = metaTypeForParameter(formal->type);
            if (!type.isValid())
                type = QMetaType::fromType<QVariant>();
            parameterTypes << type;
        }

        QMetaType returnType = metaTypeForParameter(function->returnType);
        if (!returnType.isValid())
            returnType = QMetaType::fromType<QVariant>();

        cache->appendMethod(slotName, flags, effectiveMethodIndex++, returnType, parameterNames, parameterTypes);
    }


    // Dynamic properties
    int effectiveSignalIndex = cache->signalHandlerIndexCacheStart;
    int propertyIdx = 0;
    p = obj->propertiesBegin();
    pend = obj->propertiesEnd();
    for ( ; p != pend; ++p, ++propertyIdx) {
        QMetaType propertyType;
        QTypeRevision propertyTypeVersion = QTypeRevision::zero();
        QQmlPropertyData::Flags propertyFlags;

        const QV4::CompiledData::BuiltinType type = p->builtinType();

        if (type == QV4::CompiledData::BuiltinType::Var)
            propertyFlags.type = QQmlPropertyData::Flags::VarPropertyType;

        if (type != QV4::CompiledData::BuiltinType::InvalidBuiltin) {
            propertyType = metaTypeForPropertyType(type);
        } else {
            Q_ASSERT(!p->isBuiltinType());

            QQmlType qmltype;
            bool selfReference = false;
            if (!imports->resolveType(stringAt(p->builtinTypeOrTypeNameIndex()), &qmltype, nullptr, nullptr,
                                      nullptr, QQmlType::AnyRegistrationType, &selfReference)) {
                return qQmlCompileError(p->location, QQmlPropertyCacheCreatorBase::tr("Invalid property type"));
            }

            // inline components are not necessarily valid yet
            Q_ASSERT(qmltype.isValid() || qmltype.isInlineComponentType());
            if (qmltype.isComposite() || qmltype.isInlineComponentType()) {
                CompositeMetaTypeIds typeIds;
                if (qmltype.isInlineComponentType()) {
                    auto objectId = qmltype.inlineComponentId();
                    auto containingType = qmltype.containingType();
                    if (containingType.isValid()) {
                        auto icType = containingType.lookupInlineComponentById(objectId);
                        typeIds = {icType.typeId(), icType.qListTypeId()};
                    } else {
                        typeIds = {};
                    }
                    if (!typeIds.isValid()) // type has not been registered yet, we must be in containing type
                        typeIds = objectContainer->typeIdsForComponent(objectId);
                    Q_ASSERT(typeIds.isValid());
                } else if (selfReference) {
                     typeIds = objectContainer->typeIdsForComponent();
                } else {
                    QQmlRefPointer<QQmlTypeData> tdata = enginePrivate->typeLoader.getType(qmltype.sourceUrl());
                    Q_ASSERT(tdata);
                    Q_ASSERT(tdata->isComplete());

                    auto compilationUnit = tdata->compilationUnit();
                    typeIds = compilationUnit->typeIdsForComponent();
                }

                if (p->isList()) {
                    propertyType = typeIds.listId;
                } else {
                    propertyType = typeIds.id;
                }
            } else {
                if (p->isList()) {
                    propertyType = qmltype.qListTypeId();
                } else {
                    propertyType = qmltype.typeId();
                    propertyTypeVersion = qmltype.version();
                }
            }

            if (p->isList())
                propertyFlags.type = QQmlPropertyData::Flags::QListType;
            else
                propertyFlags.type = QQmlPropertyData::Flags::QObjectDerivedType;
        }

        if (!p->isReadOnly() && !p->isList())
            propertyFlags.setIsWritable(true);


        QString propertyName = stringAt(p->nameIndex);
        if (!obj->hasAliasAsDefaultProperty() && propertyIdx == obj->indexOfDefaultPropertyOrAlias)
            cache->_defaultPropertyName = propertyName;
        cache->appendProperty(propertyName, propertyFlags, effectivePropertyIndex++,
                              propertyType, propertyTypeVersion, effectiveSignalIndex);

        effectiveSignalIndex++;
    }

    QQmlError noError;
    return noError;
}

template <typename ObjectContainer>
inline QMetaType QQmlPropertyCacheCreator<ObjectContainer>::metaTypeForParameter(const QV4::CompiledData::ParameterType &param,
                                                                           QString *customTypeName)
{
    if (param.indexIsBuiltinType()) {
        // built-in type
        return metaTypeForPropertyType(
                static_cast<QV4::CompiledData::BuiltinType>(param.typeNameIndexOrBuiltinType()));
    }

    // lazily resolved type
    const QString typeName = stringAt(param.typeNameIndexOrBuiltinType());
    if (customTypeName)
        *customTypeName = typeName;
    QQmlType qmltype;
    bool selfReference = false;
    if (!imports->resolveType(typeName, &qmltype, nullptr, nullptr, nullptr,
                              QQmlType::AnyRegistrationType, &selfReference))
        return QMetaType();

    if (!qmltype.isComposite()) {
        const QMetaType typeId = qmltype.typeId();
        if (!typeId.isValid() && qmltype.isInlineComponentType()) {
            const int objectId = qmltype.inlineComponentId();
            return objectContainer->typeIdsForComponent(objectId).id;
        } else {
            return typeId;
        }
    }

    if (selfReference)
        return objectContainer->typeIdsForComponent().id;

    QQmlRefPointer<QQmlTypeData> tdata = enginePrivate->typeLoader.getType(qmltype.sourceUrl());
    Q_ASSERT(tdata);
    Q_ASSERT(tdata->isComplete());

    auto compilationUnit = tdata->compilationUnit();

    return compilationUnit->typeIds.id;
}

template <typename ObjectContainer>
class QQmlPropertyCacheAliasCreator
{
public:
    typedef typename ObjectContainer::CompiledObject CompiledObject;

    QQmlPropertyCacheAliasCreator(QQmlPropertyCacheVector *propertyCaches, const ObjectContainer *objectContainer);

    void appendAliasPropertiesToMetaObjects(QQmlEnginePrivate *enginePriv);

    QQmlError appendAliasesToPropertyCache(const CompiledObject &component, int objectIndex, QQmlEnginePrivate *enginePriv);

private:
    void appendAliasPropertiesInMetaObjectsWithinComponent(const CompiledObject &component, int firstObjectIndex, QQmlEnginePrivate *enginePriv);
    QQmlError propertyDataForAlias(const CompiledObject &component, const QV4::CompiledData::Alias &alias, QMetaType *type, QTypeRevision *version, QQmlPropertyData::Flags *propertyFlags, QQmlEnginePrivate *enginePriv);

    void collectObjectsWithAliasesRecursively(int objectIndex, QVector<int> *objectsWithAliases) const;

    int objectForId(const CompiledObject &component, int id) const;

    QQmlPropertyCacheVector *propertyCaches;
    const ObjectContainer *objectContainer;
};

template <typename ObjectContainer>
inline QQmlPropertyCacheAliasCreator<ObjectContainer>::QQmlPropertyCacheAliasCreator(QQmlPropertyCacheVector *propertyCaches, const ObjectContainer *objectContainer)
    : propertyCaches(propertyCaches)
    , objectContainer(objectContainer)
{

}

template <typename ObjectContainer>
inline void QQmlPropertyCacheAliasCreator<ObjectContainer>::appendAliasPropertiesToMetaObjects(QQmlEnginePrivate *enginePriv)
{
    // skip the root object (index 0) as that one does not have a first object index originating
    // from a binding.
    for (int i = 1; i < objectContainer->objectCount(); ++i) {
        const CompiledObject &component = *objectContainer->objectAt(i);
        if (!component.hasFlag(QV4::CompiledData::Object::IsComponent))
            continue;

        const auto rootBinding = component.bindingsBegin();
        appendAliasPropertiesInMetaObjectsWithinComponent(component, rootBinding->value.objectIndex, enginePriv);
    }

    const int rootObjectIndex = 0;
    appendAliasPropertiesInMetaObjectsWithinComponent(*objectContainer->objectAt(rootObjectIndex), rootObjectIndex, enginePriv);
}

template <typename ObjectContainer>
inline void QQmlPropertyCacheAliasCreator<ObjectContainer>::appendAliasPropertiesInMetaObjectsWithinComponent(const CompiledObject &component, int firstObjectIndex, QQmlEnginePrivate *enginePriv)
{
    QVector<int> objectsWithAliases;
    collectObjectsWithAliasesRecursively(firstObjectIndex, &objectsWithAliases);
    if (objectsWithAliases.isEmpty())
        return;

    const auto allAliasTargetsExist = [this, &component](const CompiledObject &object) {
        auto alias = object.aliasesBegin();
        auto end = object.aliasesEnd();
        for ( ; alias != end; ++alias) {
            Q_ASSERT(alias->hasFlag(QV4::CompiledData::Alias::Resolved));

            const int targetObjectIndex = objectForId(component, alias->targetObjectId());
            Q_ASSERT(targetObjectIndex >= 0);

            if (alias->isAliasToLocalAlias())
                continue;

            if (alias->encodedMetaPropertyIndex == -1)
                continue;

            const QQmlPropertyCache *targetCache = propertyCaches->at(targetObjectIndex);
            Q_ASSERT(targetCache);

            int coreIndex = QQmlPropertyIndex::fromEncoded(alias->encodedMetaPropertyIndex).coreIndex();
            QQmlPropertyData *targetProperty = targetCache->property(coreIndex);
            if (!targetProperty)
                return false;
       }
       return true;
    };

    do {
        QVector<int> pendingObjects;

        for (int objectIndex: qAsConst(objectsWithAliases)) {
            const CompiledObject &object = *objectContainer->objectAt(objectIndex);

            if (allAliasTargetsExist(object)) {
                appendAliasesToPropertyCache(component, objectIndex, enginePriv);
            } else {
                pendingObjects.append(objectIndex);
            }

        }
        qSwap(objectsWithAliases, pendingObjects);
    } while (!objectsWithAliases.isEmpty());
}

template <typename ObjectContainer>
inline void QQmlPropertyCacheAliasCreator<ObjectContainer>::collectObjectsWithAliasesRecursively(int objectIndex, QVector<int> *objectsWithAliases) const
{
    const CompiledObject &object = *objectContainer->objectAt(objectIndex);
    if (object.aliasCount() > 0)
        objectsWithAliases->append(objectIndex);

    // Stop at Component boundary
    if (object.hasFlag(QV4::CompiledData::Object::IsComponent) && objectIndex != /*root object*/0)
        return;

    auto binding = object.bindingsBegin();
    auto end = object.bindingsEnd();
    for (; binding != end; ++binding) {
        switch (binding->type()) {
        case QV4::CompiledData::Binding::Type_Object:
        case QV4::CompiledData::Binding::Type_AttachedProperty:
        case QV4::CompiledData::Binding::Type_GroupProperty:
            collectObjectsWithAliasesRecursively(binding->value.objectIndex, objectsWithAliases);
            break;
        default:
            break;
        }
    }
}

template <typename ObjectContainer>
inline QQmlError QQmlPropertyCacheAliasCreator<ObjectContainer>::propertyDataForAlias(
        const CompiledObject &component, const QV4::CompiledData::Alias &alias, QMetaType *type,
        QTypeRevision *version, QQmlPropertyData::Flags *propertyFlags,
        QQmlEnginePrivate *enginePriv)
{
    *type = QMetaType();
    bool writable = false;
    bool resettable = false;
    bool bindable = false;

    propertyFlags->setIsAlias(true);

    if (alias.isAliasToLocalAlias()) {
        const QV4::CompiledData::Alias *lastAlias = &alias;
        QVarLengthArray<const QV4::CompiledData::Alias *, 4> seenAliases({lastAlias});

        do {
            const int targetObjectIndex = objectForId(component, lastAlias->targetObjectId());
            Q_ASSERT(targetObjectIndex >= 0);
            const CompiledObject *targetObject = objectContainer->objectAt(targetObjectIndex);
            Q_ASSERT(targetObject);

            auto nextAlias = targetObject->aliasesBegin();
            for (uint i = 0; i < lastAlias->localAliasIndex; ++i)
                ++nextAlias;

            const QV4::CompiledData::Alias *targetAlias = &(*nextAlias);
            if (seenAliases.contains(targetAlias)) {
                return qQmlCompileError(targetAlias->location,
                                        QQmlPropertyCacheCreatorBase::tr("Cyclic alias"));
            }

            seenAliases.append(targetAlias);
            lastAlias = targetAlias;
        } while (lastAlias->isAliasToLocalAlias());

        return propertyDataForAlias(component, *lastAlias, type, version, propertyFlags, enginePriv);
    }

    const int targetObjectIndex = objectForId(component, alias.targetObjectId());
    Q_ASSERT(targetObjectIndex >= 0);
    const CompiledObject &targetObject = *objectContainer->objectAt(targetObjectIndex);

    if (alias.encodedMetaPropertyIndex == -1) {
        Q_ASSERT(alias.hasFlag(QV4::CompiledData::Alias::AliasPointsToPointerObject));
        auto *typeRef = objectContainer->resolvedType(targetObject.inheritedTypeNameIndex);
        if (!typeRef) {
            // Can be caused by the alias target not being a valid id or property. E.g.:
            // property alias dataValue: dataVal
            // invalidAliasComponent { id: dataVal }
            return qQmlCompileError(targetObject.location,
                                    QQmlPropertyCacheCreatorBase::tr("Invalid alias target"));
        }

        const auto referencedType = typeRef->type();
        if (referencedType.isValid()) {
            *type = referencedType.typeId();
            if (!type->isValid() && referencedType.isInlineComponentType()) {
                int objectId = referencedType.inlineComponentId();
                *type = objectContainer->typeIdsForComponent(objectId).id;
                Q_ASSERT(type->isValid());
            }
        } else {
            *type = typeRef->compilationUnit()->typeIds.id;
        }

        *version = typeRef->version();

        propertyFlags->type = QQmlPropertyData::Flags::QObjectDerivedType;
    } else {
        int coreIndex = QQmlPropertyIndex::fromEncoded(alias.encodedMetaPropertyIndex).coreIndex();
        int valueTypeIndex = QQmlPropertyIndex::fromEncoded(alias.encodedMetaPropertyIndex).valueTypeIndex();

        QQmlPropertyCache *targetCache = propertyCaches->at(targetObjectIndex);
        Q_ASSERT(targetCache);

        QQmlPropertyData *targetProperty = targetCache->property(coreIndex);
        Q_ASSERT(targetProperty);

        const QMetaType targetPropType = targetProperty->propType();

        const auto resolveType = [](QMetaType targetPropType) {
            if (targetPropType.flags() & QMetaType::IsEnumeration)
                return QMetaType::fromType<int>();
            else
                return targetPropType;
        };

        const auto populateWithPropertyData = [&](const QQmlPropertyData *property) {
            *type = resolveType(property->propType());
            writable = property->isWritable();
            resettable = property->isResettable();
            bindable = property->isBindable();

            if (property->isVarProperty())
                propertyFlags->type = QQmlPropertyData::Flags::QVariantType;
            else
                propertyFlags->copyPropertyTypeFlags(property->flags());
        };

        // for deep aliases, valueTypeIndex is always set
        if (!QQmlMetaType::isValueType(targetPropType) && valueTypeIndex != -1) {
            // deep alias property

            QQmlRefPointer<QQmlPropertyCache> typeCache
                    = enginePriv->propertyCacheForType(targetPropType.id());

            if (!typeCache) {
                // See if it's a half-resolved composite type
                if (const QV4::ResolvedTypeReference *typeRef
                        = objectContainer->resolvedType(targetPropType)) {
                    typeCache = typeRef->typePropertyCache();
                }
            }

            const QQmlPropertyData *typeProperty = typeCache
                    ? typeCache->property(valueTypeIndex)
                    : nullptr;
            if (typeProperty == nullptr) {
                return qQmlCompileError(
                        alias.referenceLocation,
                        QQmlPropertyCacheCreatorBase::tr("Invalid alias target"));
            }
            populateWithPropertyData(typeProperty);
        } else {
            // value type or primitive type or enum
            populateWithPropertyData(targetProperty);

            if (valueTypeIndex != -1) {
                const QMetaObject *valueTypeMetaObject
                        = QQmlMetaType::metaObjectForValueType(*type);
                const QMetaProperty valueTypeMetaProperty
                        = valueTypeMetaObject->property(valueTypeIndex);
                *type = resolveType(valueTypeMetaProperty.metaType());

                // We can only write or reset the value type property if we can write
                // the value type itself.
                resettable = writable && valueTypeMetaProperty.isResettable();
                writable = writable && valueTypeMetaProperty.isWritable();

                bindable = valueTypeMetaProperty.isBindable();
            }
        }
    }

    propertyFlags->setIsWritable(
            writable && !alias.hasFlag(QV4::CompiledData::Alias::IsReadOnly));
    propertyFlags->setIsResettable(resettable);
    propertyFlags->setIsBindable(bindable);
    return QQmlError();
}

template <typename ObjectContainer>
inline QQmlError QQmlPropertyCacheAliasCreator<ObjectContainer>::appendAliasesToPropertyCache(
        const CompiledObject &component, int objectIndex, QQmlEnginePrivate *enginePriv)
{
    const CompiledObject &object = *objectContainer->objectAt(objectIndex);
    if (!object.aliasCount())
        return QQmlError();

    QQmlPropertyCache *propertyCache = propertyCaches->at(objectIndex);
    Q_ASSERT(propertyCache);

    int effectiveSignalIndex = propertyCache->signalHandlerIndexCacheStart + propertyCache->propertyIndexCache.count();
    int effectivePropertyIndex = propertyCache->propertyIndexCacheStart + propertyCache->propertyIndexCache.count();

    int aliasIndex = 0;
    auto alias = object.aliasesBegin();
    auto end = object.aliasesEnd();
    for ( ; alias != end; ++alias, ++aliasIndex) {
        Q_ASSERT(alias->hasFlag(QV4::CompiledData::Alias::Resolved));

        QMetaType type;
        QTypeRevision version = QTypeRevision::zero();
        QQmlPropertyData::Flags propertyFlags;
        QQmlError error = propertyDataForAlias(component, *alias, &type, &version,
                                               &propertyFlags, enginePriv);
        if (error.isValid())
            return error;

        const QString propertyName = objectContainer->stringAt(alias->nameIndex());

        if (object.hasAliasAsDefaultProperty() && aliasIndex == object.indexOfDefaultPropertyOrAlias)
            propertyCache->_defaultPropertyName = propertyName;

        propertyCache->appendProperty(propertyName, propertyFlags, effectivePropertyIndex++,
                                      type, version, effectiveSignalIndex++);
    }

    return QQmlError();
}

template <typename ObjectContainer>
inline int QQmlPropertyCacheAliasCreator<ObjectContainer>::objectForId(const CompiledObject &component, int id) const
{
    for (quint32 i = 0, count = component.namedObjectsInComponentCount(); i < count; ++i) {
        const int candidateIndex = component.namedObjectsInComponentTable()[i];
        const CompiledObject &candidate = *objectContainer->objectAt(candidateIndex);
        if (candidate.objectId() == id)
            return candidateIndex;
    }
    return -1;
}

QT_END_NAMESPACE

#endif // QQMLPROPERTYCACHECREATOR_P_H
