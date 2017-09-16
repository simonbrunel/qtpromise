import QtQuick 2.3
import QtPromise 1.0
import QtTest 1.0

TestCase {
    name: "Extension"

    function test_global() {
        compare(typeof __qtpromise_private__, 'undefined');
        compare(typeof Promise, 'function');
        compare(typeof Promise.resolve, 'function');
        compare(typeof Promise.reject, 'function');
    }

    function test_instance() {
        var p = new Promise(function() {});
        compare(Object.prototype.toString(p), '[object Object]');
        compare(p instanceof Promise, true);
        compare(typeof p, 'object');
    }

    function test_prototype() {
        var p = new Promise(function() {});

        [
            'delay',
            'fail',
            'finally',
            'isFulfilled',
            'isRejected',
            'isPending',
            'tap',
            'then',
            'wait',
        ].forEach(function(name) {
            compare(typeof p[name], 'function');
        });
    }
}
