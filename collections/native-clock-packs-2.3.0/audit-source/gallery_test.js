const assert = require('node:assert/strict');
const fs = require('node:fs');
const vm = require('node:vm');
const elements = new Map();
const document = {querySelector(selector) {
  if (!elements.has(selector)) elements.set(selector, {
    value: selector === '#library' ? 'all' : '',
    textContent: selector === '#gallery-data' ? '{"scenes":[],"artwork":[]}' : ''
  });
  return elements.get(selector);
}};
const context = vm.createContext({document, performance: {now: () => 0},
  requestAnimationFrame() {}});
vm.runInContext(fs.readFileSync(process.argv[2], 'utf8'), context);
for (const [elapsed, minute, start, next] of [
  [59960, 754, null, 60000], [60000, 755, 60000, 120000],
  [119960, 755, 60000, 120000], [120000, 756, 120000, 180000]
]) {
  document.querySelector('#seek').oninput({target: {value: String(elapsed)}});
  assert.equal(vm.runInContext('minuteOfDay', context), minute);
  assert.equal(vm.runInContext('actionStart', context), start);
  assert.equal(vm.runInContext('auto', context), next);
  if (elapsed % 60000 === 0) {
    for (const mode of ['minute', 'classic']) {
      assert.equal(vm.runInContext(`frame({mode:'${mode}', action:[300,60], frame_ms:40})`, context), 300);
    }
  }
}
document.querySelector('#step').onclick();
assert.equal(vm.runInContext('minuteOfDay', context), 756);
assert.equal(vm.runInContext('actionStart', context), 120000);
assert.equal(vm.runInContext('elapsed', context), 120040);
console.log('Gallery seek/minute/step boundaries passed');
