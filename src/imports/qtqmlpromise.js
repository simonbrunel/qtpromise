.pragma library

(function(global) {
    var private = global.__qtpromise_private__;
    delete global.__qtpromise_private__;

    var Promise = global.Promise = function(resolver) {
        return private.create(function(proxy) {
            resolver(proxy.resolve, proxy.reject);
        }, this);
    };

    ['all', 'reject', 'resolve'].forEach(function(method) {
        Promise[method] = private[method];
    });
})(this);
