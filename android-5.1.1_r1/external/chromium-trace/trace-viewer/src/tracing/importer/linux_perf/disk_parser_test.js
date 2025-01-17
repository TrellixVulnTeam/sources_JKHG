// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

base.require('tracing.test_utils');
base.require('tracing.importer.linux_perf_importer');

base.unittest.testSuite('tracing.importer.linux_perf.disk_parser', function() {
  test('diskImport', function() {
    var lines = [
      // NB: spliced from different traces; mismatched timestamps don't matter
      'AsyncTask #2-18830 [000] ...1 154578.668286: ext4_sync_file_enter: ' +
          'dev 259,1 ino 81993 parent 81906 datasync 1',
      'Binder_A-3179 [001] ...1  1354.510088: f2fs_sync_file_enter: ' +
          'dev = (259,14), ino = 4882, pino = 313, i_mode = 0x81b0, i_size = ' +
          '25136, i_nlink = 1, i_blocks = 8, i_advise = 0x0',
      'Binder_A-3179 [001] ...1  1354.514013: f2fs_sync_file_exit: ' +
          'dev = (259,14), ino = 4882, checkpoint is not needed, datasync = 1, ret = 0',
      'mmcqd/0-81    [000] d..2 154578.668390: block_rq_issue: ' +
          '179,0 WS 0 () 3427120 + 16 [mmcqd/0]',
      'mmcqd/0-81    [000] d..2 154578.669181: block_rq_complete: ' +
          '179,0 WS () 3427120 + 16 [0]',
      'mmcqd/0-81    [001] d..2 154578.670853: block_rq_issue: ' +
          '179,0 FWS 0 () 18446744073709551615 + 0 [mmcqd/0]',
      'mmcqd/0-81    [001] d..2 154578.670869: block_rq_complete: ' +
          '179,0 FWS () 18446744073709551615 + 0 [0]',
      'AsyncTask #2-18830 [001] ...1 154578.670901: ext4_sync_file_exit: ' +
          'dev 259,1 ino 81993 ret 0',
      'mmcqd/0-81    [001] d..2 154578.877038: block_rq_issue: ' +
          '179,0 R 0 () 3255256 + 8 [mmcqd/0]',
      'mmcqd/0-81    [001] d..2 154578.877110: block_rq_issue: ' +
          '179,0 R 0 () 3255288 + 8 [mmcqd/0]',
      'mmcqd/0-81    [000] d..2 154578.877345: block_rq_complete: ' +
          '179,0 R () 3255256 + 8 [0]',
      'mmcqd/0-81    [000] d..2 154578.877466: block_rq_complete: ' +
          '179,0 R () 3255288 + 8 [0]',
      'ContactsProvide-1184 [000] ...1 66.613719: f2fs_write_begin: ' +
          'dev = (253,2), ino = 3342, pos = 0, len = 75, flags = 0',
      'ContactsProvide-1184 [000] ...1 66.613733: f2fs_write_end: ' +
          'dev = (253,2), ino = 3342, pos = 0, len = 75, copied = 75'
    ];
    var m = new tracing.TraceModel(lines.join('\n'), false);
    assertFalse(m.hasImportWarnings);

    var blockThread = undefined;
    var ext4Thread = undefined;
    var f2fsSyncThread = undefined;
    var f2fsWriteThread = undefined;

    m.getAllThreads().forEach(function(t) {
      switch (t.name) {
        case 'block:mmcqd/0':
          blockThread = t;
          break;
        case 'ext4:AsyncTask #2':
          ext4Thread = t;
          break;
        case 'f2fs:Binder_A':
          f2fsSyncThread = t;
          break;
        case 'f2fs:ContactsProvide':
          f2fsWriteThread = t;
          break;
        default:
          throw new unittest.TestError('Unexpected thread named ' + t.name);
      }
    });

    assertNotUndefined(blockThread);
    assertNotUndefined(ext4Thread);
    assertNotUndefined(f2fsSyncThread);
    assertNotUndefined(f2fsWriteThread);

    assertEquals(4, blockThread.asyncSliceGroup.length);
    var slice = blockThread.asyncSliceGroup.slices[0];
    assertEquals('block', slice.category);
    assertEquals('write sync', slice.title);
    assertEquals('179,0', slice.args.device);
    assertEquals(0, slice.args.error);
    assertEquals(16, slice.args.numSectors);
    assertEquals(3427120, slice.args.sector);
    assertEquals(1, ext4Thread.asyncSliceGroup.length);

    slice = ext4Thread.asyncSliceGroup.slices[0];
    assertEquals('ext4', slice.category);
    assertEquals('fdatasync', slice.title);
    assertEquals('259,1', slice.args.device);
    assertEquals(0, slice.args.error);
    assertEquals(81993, slice.args.inode);
    assertEquals(1, f2fsSyncThread.asyncSliceGroup.length);

    slice = f2fsSyncThread.asyncSliceGroup.slices[0];
    assertEquals('f2fs', slice.category);
    assertEquals('fsync', slice.title);
    assertEquals('259,14', slice.args.device);
    assertEquals(0, slice.args.error);
    assertEquals(4882, slice.args.inode);
    assertEquals(1, f2fsWriteThread.asyncSliceGroup.length);

    slice = f2fsWriteThread.asyncSliceGroup.slices[0];
    assertEquals('f2fs', slice.category);
    assertEquals('f2fs_write', slice.title);
    assertEquals('253,2', slice.args.device);
    assertEquals(3342, slice.args.inode);
    assertEquals(false, slice.args.error);
  });
});
