/* global Module, craftInvokerFunction: writable, whenDependentTypesAreResolved: writable, initRuntime: writable */

// Some signatures are resolved later, while registering an unrelated type.
// Carry the original registration stack through Embind's deferred callbacks.
let registrationStack
const originalInitRuntime = initRuntime
initRuntime = function () {
    const originalResolveTypes = whenDependentTypesAreResolved
    whenDependentTypesAreResolved = function (types, dependencies, resolve) {
        const origin = registrationStack || new Error().stack
        return originalResolveTypes(types, dependencies, function (...args) {
            const previous = registrationStack
            registrationStack = origin
            try {
                return resolve.apply(this, args)
            } finally {
                registrationStack = previous
            }
        })
    }
    return originalInitRuntime()
}

// Wrap Embind's invoker factory before registration, not public functions after
// initialization: this covers constructors and each overload without changing
// overload resolution, prototypes, or function metadata used by Embind.
Module.__otioBindingCoverage = []
const originalCraftInvoker = craftInvokerFunction
craftInvokerFunction = function (name, types, classType, cppInvoker, target, isAsync) {
    const record = {
        name,
        arguments: types.slice(2).map(type => type.name),
        returns: types[0].name,
        registrationStack: registrationStack || new Error().stack,
        calls: 0,
        cppCalls: 0,
        completed: 0,
        threw: 0,
        argumentCounts: {},
    }
    Module.__otioBindingCoverage.push(record)
    const invoker = originalCraftInvoker(name, types, classType, function (...args) {
        // Embind has converted arguments and validated the receiver before this.
        record.cppCalls++
        return cppInvoker.apply(this, args)
    }, target, isAsync)
    const tracked = function (...args) {
        record.calls++
        record.argumentCounts[args.length] = (record.argumentCounts[args.length] || 0) + 1
        try {
            const result = invoker.apply(this, args)
            record.completed++
            return result
        } catch (error) {
            record.threw++
            throw error
        }
    }
    Object.assign(tracked, invoker)
    Object.defineProperty(tracked, 'length', { value: invoker.length })
    return tracked
}
