/*
 * WARNING!
 *
 * Do not edit this file directly, it is built from the sources at
 * https://github.com/mozilla/source-map/
 */

Components.utils.import('resource://test/Utils.jsm');
/* -*- Mode: js; js-indent-level: 2; -*- */
/*
 * Copyright 2011 Mozilla Foundation and contributors
 * Licensed under the New BSD license. See LICENSE or:
 * http://opensource.org/licenses/BSD-3-Clause
 */
define("test/source-map/test-binary-search", ["require", "exports", "module"], function (require, exports, module) {

  var binarySearch = require('source-map/binary-search');

  function numberCompare(a, b) {
    return a - b;
  }

  exports['test too high'] = function (assert, util) {
    var needle = 30;
    var haystack = [2,4,6,8,10,12,14,16,18,20];

    assert.doesNotThrow(function () {
      binarySearch.search(needle, haystack, numberCompare);
    });

    assert.equal(binarySearch.search(needle, haystack, numberCompare), 20);
  };

  exports['test too low'] = function (assert, util) {
    var needle = 1;
    var haystack = [2,4,6,8,10,12,14,16,18,20];

    assert.doesNotThrow(function () {
      binarySearch.search(needle, haystack, numberCompare);
    });

    assert.equal(binarySearch.search(needle, haystack, numberCompare), null);
  };

  exports['test exact search'] = function (assert, util) {
    var needle = 4;
    var haystack = [2,4,6,8,10,12,14,16,18,20];

    assert.equal(binarySearch.search(needle, haystack, numberCompare), 4);
  };

  exports['test fuzzy search'] = function (assert, util) {
    var needle = 19;
    var haystack = [2,4,6,8,10,12,14,16,18,20];

    assert.equal(binarySearch.search(needle, haystack, numberCompare), 18);
  };

});
function run_test() {
  runSourceMapTests('test/source-map/test-binary-search', do_throw);
}
