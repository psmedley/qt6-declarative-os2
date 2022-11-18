// This file was procedurally generated from the following sources:
// - src/accessor-names/computed-err-to-prop-key.case
// - src/accessor-names/error/obj.template
/*---
description: Abrupt completion when coercing to property key value (Object initializer)
esid: sec-object-initializer-runtime-semantics-evaluation
es6id: 12.2.6.8
flags: [generated]
info: |
    ObjectLiteral :
      { PropertyDefinitionList }
      { PropertyDefinitionList , }

    1. Let obj be ObjectCreate(%ObjectPrototype%).
    2. Let status be the result of performing PropertyDefinitionEvaluation of
       PropertyDefinitionList with arguments obj and true.

    12.2.6.7 Runtime Semantics: Evaluation

    [...]

    ComputedPropertyName : [ AssignmentExpression ]

    1. Let exprValue be the result of evaluating AssignmentExpression.
    2. Let propName be ? GetValue(exprValue).
    3. Return ? ToPropertyKey(propName).

    7.1.14 ToPropertyKey

    1. Let key be ? ToPrimitive(argument, hint String).

    7.1.1 ToPrimitive

    [...]
    7. Return ? OrdinaryToPrimitive(input, hint).

    7.1.1.1 OrdinaryToPrimitive

    5. For each name in methodNames in List order, do
       [...]
    6. Throw a TypeError exception.
---*/
var badKey = Object.create(null);


assert.throws(TypeError, function() {
  ({
    get [badKey]() {}
  });
}, '`get` accessor');

assert.throws(TypeError, function() {
  ({
    set [badKey](_) {}
  });
}, '`set` accessor');
