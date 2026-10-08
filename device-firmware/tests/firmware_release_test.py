import importlib.util
import unittest
from pathlib import Path

s = importlib.util.spec_from_file_location('release', Path(__file__).resolve().parents[1] / 'scripts/firmware-release.py')
m = importlib.util.module_from_spec(s)
s.loader.exec_module(m)

class ReleaseTest(unittest.TestCase):
    def setUp(self):
        self.binary = bytearray(512)
        self.binary[0] = 0xE9
        self.binary[32:36] = bytes.fromhex('3254cdab')
        self.binary[48:56] = b'2026.8.2'
        self.binary[200:210] = b'sntl-2.1.0'
        self.url = 'https://updates.example/releases/sntl-2.1.0.bin'
    def make(self, **kwargs):
        args = dict(binary=bytes(self.binary), version='sntl-2.1.0', firmware_url=self.url,
                    release_url='https://github.com/example/clock/releases/tag/sntl-2.1.0', summary='Test')
        args.update(kwargs)
        return m.manifest(**args)
    def test_valid(self):
        ota = self.make()['builds'][0]['ota']
        self.assertEqual(ota['size'], 512)
        self.assertEqual(len(ota['md5']), 32)
        self.assertEqual(len(ota['sha256']), 64)
    def test_reject_credentials_and_plaintext(self):
        for url in ['http://updates.example/sntl-2.1.0.bin', 'https://user:secret@updates.example/sntl-2.1.0.bin', self.url + '?token=secret']:
            with self.subTest(url=url), self.assertRaises(ValueError): self.make(firmware_url=url)
    def test_reject_wrong_artifact_or_version(self):
        for args in [dict(binary=b'\0'*512), dict(binary=bytes(self.binary)+b'\0'*m.MAX_OTA_BYTES), dict(version='sntl-2.1.1'), dict(firmware_url='https://updates.example/latest.bin')]:
            with self.subTest(args=list(args)), self.assertRaises(ValueError): self.make(**args)

if __name__ == '__main__': unittest.main()
