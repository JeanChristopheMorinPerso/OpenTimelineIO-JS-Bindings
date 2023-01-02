const factory = require('../install/opentime');
const { expect, test, beforeAll } = require('@jest/globals');

/**
 * @type {factory.CustomEmbindModule}
 */
let lib;

beforeAll(async () => {
    lib = await factory();
});

test('create', () => {
    let t_val = 30.2;
    const t = new lib.RationalTime(t_val);
    expect(t).toBeDefined();
    expect(t.value).toEqual(t_val);

    t_val = -30.2;
    const t2 = new lib.RationalTime(t_val);
    expect(t2).toBeDefined();
    expect(t2.value).toEqual(t_val);

    t_val = -30.2;
    const t3 = new lib.RationalTime(t_val, 200.0);
    expect(t3).toBeDefined();
    expect(t3.value).toEqual(t_val);
    // TODO: Why is the float lost?
    expect(t3.rate).toEqual(200);

    const t4 = new lib.RationalTime()
    expect(t4.value).toEqual(0)
    expect(t4.rate).toEqual(1.0)
});

test('string', () => {
    t1 = new lib.RationalTime(0.0, 2.0)
    expect(t1.toString()).toEqual('RationalTime(value=0, rate=2)')

    t2 = new lib.RationalTime(10.5, 24.8)
    expect(t2.toString()).toEqual('RationalTime(value=10.5, rate=24.8)')
})

test('equality', () => {
    const t1 = new lib.RationalTime(30.1)
    expect(t1).toEqual(t1)

    const t2 = new lib.RationalTime(30.1)
    expect(t2).not.toBe(t1)
    expect(t2).toEqual(t1)
});

test('inequality', () => {
    t1 = new lib.RationalTime(30.2)
    t2 = new lib.RationalTime(33.2)
    expect(t1 !== t2).toBe(true)
    expect(t1 != t2).toBe(true)

    const t3 = new lib.RationalTime(30.2)
    expect(t3 !== t1).toBe(false)
})
//     def test_inequality(self):
//         t1 = otio.opentime.RationalTime(30.2)
//         self.assertEqual(t1, t1)
//         t2 = otio.opentime.RationalTime(33.2)
//         self.assertTrue(t1 is not t2)
//         self.assertNotEqual(t1, t2)
//         t3 = otio.opentime.RationalTime(30.2)
//         self.assertTrue(t1 is not t3)
//         self.assertFalse(t1 != t3)

test('comparison', () => {
    t1 = new lib.RationalTime(15.2)
    t2 = new lib.RationalTime(15.6)
    console.log(t1.valueOf())
    expect(t1 < t2).toBe(true)
    expect(t1 <= t2).toBe(true)
    expect(t1 > t2).toBe(false)
    expect(t1 >= t2).toBe(false)

    // Ensure the equality case of the comparisons works correctly
    t3 = new lib.RationalTime(30.4, 2)
    expect(t1 <= t3).toBe(true)
    expect(t1 >= t3).toBe(true)
    expect(t3 <= t1).toBe(true)
    expect(t3 >= t1).toBe(true)

    // test implicit base conversion
    t2 = new lib.RationalTime(15.6, 48)
    expect(t1 > t2).toBe(true)
    expect(t1 >= t2).toBe(true)
    expect(t1 < t2).toBe(false)
    expect(t1 <= t2).toBe(false)
})

//     def test_copy(self):
//         t1 = otio.opentime.RationalTime(18, 24)

//         t2 = copy.copy(t1)
//         self.assertEqual(t2, otio.opentime.RationalTime(18, 24))

//     def test_deepcopy(self):
//         t1 = otio.opentime.RationalTime(18, 24)

//         t2 = copy.deepcopy(t1)
//         self.assertEqual(t2, otio.opentime.RationalTime(18, 24))

//     def test_base_conversion(self):
//         # from a number
//         t = otio.opentime.RationalTime(10, 24)
//         with self.assertRaises(TypeError):
//             t.rescaled_to("foo")
//         self.assertEqual(t.rate, 24)
//         t = t.rescaled_to(48)
//         self.assertEqual(t.rate, 48)

//         # from another RationalTime
//         t = otio.opentime.RationalTime(10, 24)
//         t2 = otio.opentime.RationalTime(20, 48)
//         t = t.rescaled_to(t2)
//         self.assertEqual(t.rate, t2.rate)

//     def test_time_timecode_convert(self):
//         timecode = "00:06:56:17"
//         t = otio.opentime.from_timecode(timecode, 24)
//         self.assertEqual(timecode, otio.opentime.to_timecode(t))

//     def test_negative_timecode(self):
//         with self.assertRaises(ValueError):
//             otio.opentime.from_timecode('-01:00:13:13', 24)

//     def test_bogus_timecode(self):
//         with self.assertRaises(ValueError):
//             otio.opentime.from_timecode('pink elephants', 13)

//     def test_time_timecode_convert_bad_rate(self):
//         with self.assertRaises(ValueError) as exception_manager:
//             otio.opentime.from_timecode('01:00:13:24', 24)

//         exc_message = str(exception_manager.exception)
//         self.assertEqual(
//             exc_message,
//             "Frame rate mismatch.  Timecode '01:00:13:24' has frames beyond 23",
//         )

//     def test_timecode_24(self):
//         timecode = "00:00:01:00"
//         t = otio.opentime.RationalTime(value=24, rate=24)
//         self.assertEqual(t, otio.opentime.from_timecode(timecode, 24))

//         timecode = "00:01:00:00"
//         t = otio.opentime.RationalTime(value=24 * 60, rate=24)
//         self.assertEqual(t, otio.opentime.from_timecode(timecode, 24))

//         timecode = "01:00:00:00"
//         t = otio.opentime.RationalTime(value=24 * 60 * 60, rate=24)
//         self.assertEqual(t, otio.opentime.from_timecode(timecode, 24))

//         timecode = "24:00:00:00"
//         t = otio.opentime.RationalTime(value=24 * 60 * 60 * 24, rate=24)
//         self.assertEqual(t, otio.opentime.from_timecode(timecode, 24))

//         timecode = "23:59:59:23"
//         t = otio.opentime.RationalTime(value=24 * 60 * 60 * 24 - 1, rate=24)
//         self.assertEqual(t, otio.opentime.from_timecode(timecode, 24))

//     def test_plus_equals(self):
//         sum1 = otio.opentime.RationalTime()
//         sum2 = otio.opentime.RationalTime()

//         for i in range(10):
//             incr = otio.opentime.RationalTime(i + 1, 24)
//             sum1 += incr
//             sum2 = sum2 + incr

//         self.assertEqual(sum1, sum2)

//     def test_time_timecode_zero(self):
//         t = otio.opentime.RationalTime()
//         timecode = "00:00:00:00"
//         self.assertEqual(timecode, otio.opentime.to_timecode(t, 24))
//         self.assertEqual(t, otio.opentime.from_timecode(timecode, 24))

//     def test_long_running_timecode_24(self):
//         final_frame_number = 24 * 60 * 60 * 24 - 1
//         final_time = otio.opentime.from_frames(final_frame_number, 24)
//         self.assertEqual(
//             otio.opentime.to_timecode(final_time),
//             "23:59:59:23"
//         )

//         step_time = otio.opentime.RationalTime(value=1, rate=24)

//         # fetching this test function from the c++ module directly
//         cumulative_time = otio._opentime._testing.add_many(
//             step_time,
//             final_frame_number
//         )
//         self.assertEqual(cumulative_time, final_time)

//         # Adding by a non-multiple of 24
//         for fnum in range(1113, final_frame_number, 1113):
//             rt = otio.opentime.from_frames(fnum, 24)
//             tc = otio.opentime.to_timecode(rt)
//             rt2 = otio.opentime.from_timecode(tc, 24)
//             self.assertEqual(rt, rt2)
//             self.assertEqual(tc, otio.opentime.to_timecode(rt2))

//     def test_timecode_23976_fps(self):
//         # This should behave exactly like 24 fps
//         timecode = "00:00:01:00"
//         t = otio.opentime.RationalTime(value=24, rate=23.976)
//         self.assertEqual(t, otio.opentime.from_timecode(timecode, 23.976))

//         timecode = "00:01:00:00"
//         t = otio.opentime.RationalTime(value=24 * 60, rate=23.976)
//         self.assertEqual(t, otio.opentime.from_timecode(timecode, 23.976))

//         timecode = "01:00:00:00"
//         t = otio.opentime.RationalTime(value=24 * 60 * 60, rate=23.976)
//         self.assertEqual(t, otio.opentime.from_timecode(timecode, 23.976))

//         timecode = "24:00:00:00"
//         t = otio.opentime.RationalTime(value=24 * 60 * 60 * 24, rate=23.976)
//         self.assertEqual(t, otio.opentime.from_timecode(timecode, 23.976))

//         timecode = "23:59:59:23"
//         t = otio.opentime.RationalTime(
//             value=24 * 60 * 60 * 24 - 1,
//             rate=(24000 / 1001.0)
//         )
//         self.assertEqual(
//             t, otio.opentime.from_timecode(timecode, (24000 / 1001.0))
//         )

//     def test_converting_negative_values_to_timecode(self):
//         t = otio.opentime.RationalTime(value=-1, rate=25)
//         with self.assertRaises(ValueError):
//             otio.opentime.to_timecode(t, 25)

//     def test_dropframe_timecode_2997fps(self):
//         """Test drop frame in action. Focused on minute roll overs
//         We nominal_fps 30 for frame calculation
//         For this frame rate we drop 2 frames per minute execpt every 10th.
//         Compensation is calculated like this when below 10 minutes:
//           (fps * seconds + frames - dropframes * (minutes - 1))
//         Like this when not a whole 10 minute above 10 minutes:
//           --minutes == minutes - 1
//           (fps * seconds + frames - dropframes * (--minutes - --minutes / 10))
//         And like this after that:
//           (fps * seconds + frames - dropframes * (minutes - minutes / 10))
//         """
//         test_values = {
//             'first_four_frames': [
//                 (0, '00:00:00;00'),
//                 (1, '00:00:00;01'),
//                 (2, '00:00:00;02'),
//                 (3, '00:00:00;03')
//             ],
