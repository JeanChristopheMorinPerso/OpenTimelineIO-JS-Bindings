const factory = require('../install/opentime');
const { describe, expect, test, beforeAll } = require('@jest/globals');

/**
 * @type {factory.CustomEmbindModule}
 */
let lib;

beforeAll(async () => {
    lib = await factory();
});

test('SMPTE rate selection does not accept or select zero', () => {
    expect(lib.RationalTime.is_valid_timecode_rate(0)).toBe(false)
    expect(lib.RationalTime.nearest_valid_timecode_rate(1)).toBe(24000 / 1001)
    expect(lib.RationalTime.nearest_valid_timecode_rate(29.97)).toBe(30000 / 1001)
})

describe.each([
    ['opentime', factory],
    ['opentimelineio', require('../install/opentimelineio')]
])('%s OpenTime APIs from 0.16–0.18', (name, createModule) => {
    let time
    beforeAll(async () => {
        time = await createModule()
    })

    test('strict equality compares both frame value and rate without rescaling', () => {
        const first = new time.RationalTime(30, 24)
        const scaled = new time.RationalTime(60, 48)
        const same = new time.RationalTime(30, 24)
        try {
            expect(first.equal(scaled)).toBe(true)
            expect(first.strictly_equal(scaled)).toBe(false)
            expect(first.strictly_equal(same)).toBe(true)
        } finally {
            first.delete()
            scaled.delete()
            same.delete()
        }
    })

    test('rounding preserves rate and rounds negative halves away from zero', () => {
        for (const [value, floor, ceil, round] of [
            [-2.5, -3, -2, -3],
            [2.5, 2, 3, 3],
            [-2.25, -3, -2, -2],
            [2.75, 2, 3, 3]
        ]) {
            const original = new time.RationalTime(value, 48)
            try {
                for (const [method, expected] of [['floor', floor], ['ceil', ceil], ['round', round]]) {
                    const rounded = original[method]()
                    try {
                        expect([rounded.value, rounded.rate]).toEqual([expected, 48])
                    } finally {
                        rounded.delete()
                    }
                }
                expect(original.value).toBe(value)
            } finally {
                original.delete()
            }
        }
    })

    test('time validity permits negative values but rejects NaN and nonpositive rates', () => {
        for (const [value, rate, valid] of [
            [-3.5, 24, true], [0, 24, true], [1, 0, false],
            [1, -24, false], [NaN, 24, false], [1, NaN, false]
        ]) {
            const rational = new time.RationalTime(value, rate)
            try {
                expect(rational.is_valid_time()).toBe(valid)
                expect(rational.is_invalid_time()).toBe(!valid)
            } finally {
                rational.delete()
            }
        }
    })

    test('numeric ranges preserve fractional frames and validate duration boundaries', () => {
        const range = new time.TimeRange(-12.5, 7.25, 48)
        const start = range.start_time
        const duration = range.duration
        try {
            expect([start.value, start.rate]).toEqual([-12.5, 48])
            expect([duration.value, duration.rate]).toEqual([7.25, 48])
        } finally {
            start.delete()
            duration.delete()
            range.delete()
        }
        for (const [start, duration, rate, valid] of [
            [-12, 0, 24, true], [-12, 7.5, 48, true],
            [12, -0.5, 24, false], [12, 1, 0, false],
            [NaN, 1, 24, false], [12, NaN, 24, false]
        ]) {
            const range = new time.TimeRange(start, duration, rate)
            try {
                expect(range.is_valid_range()).toBe(valid)
                expect(range.is_invalid_range()).toBe(!valid)
            } finally {
                range.delete()
            }
        }
        const validStart = new time.RationalTime(12, 24)
        const invalidDuration = new time.RationalTime(7, 0)
        const mixed = new time.TimeRange(validStart, invalidDuration)
        try {
            expect(mixed.is_valid_range()).toBe(false)
            expect(mixed.is_invalid_range()).toBe(true)
        } finally {
            mixed.delete()
            invalidDuration.delete()
            validStart.delete()
        }
    })

    test('preferred SMPTE helper names retain the legacy aliases', () => {
        for (const [rate, valid, nearest] of [
            [0, false, 24000 / 1001],
            [29.97, false, 30000 / 1001],
            [30000 / 1001, true, 30000 / 1001],
            [48, true, 48], [49, false, 48]
        ]) {
            expect(time.RationalTime.is_smpte_timecode_rate(rate)).toBe(valid)
            expect(time.RationalTime.is_valid_timecode_rate(rate)).toBe(valid)
            expect(time.RationalTime.nearest_smpte_timecode_rate(rate)).toBe(nearest)
            expect(time.RationalTime.nearest_valid_timecode_rate(rate)).toBe(nearest)
        }
    })

    test('nearest timecode snaps arbitrary rates and respects override and drop-frame options', () => {
        const oddRate = new time.RationalTime(260, 26)
        const fractional = new time.RationalTime(37, 24)
        const drop = new time.RationalTime(1800, 30000 / 1001)
        const negative = new time.RationalTime(-1, 24)
        try {
            expect(() => oddRate.to_timecode()).toThrow()
            expect(oddRate.to_nearest_timecode()).toBe('00:00:10:00')
            expect(fractional.to_nearest_timecode(49)).toBe('00:00:01:26')
            expect(drop.to_nearest_timecode()).toBe('00:01:00;02')
            expect(drop.to_nearest_timecode(29.97, time.IsDropFrameRate.ForceYes)).toBe('00:01:00;02')
            expect(drop.to_nearest_timecode(29.97, time.IsDropFrameRate.ForceNo)).toBe('00:01:00:00')
            expect(() => fractional.to_nearest_timecode(24, time.IsDropFrameRate.ForceYes)).toThrow()
            expect(() => negative.to_nearest_timecode()).toThrow()
        } finally {
            oddRate.delete()
            fractional.delete()
            drop.delete()
            negative.delete()
        }
    })
})

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
    const t1 = new lib.RationalTime(0.0, 2.0)
    expect(t1.toString()).toEqual('[object RationalTime]')

    const t2 = new lib.RationalTime(10.5, 24.8)
    expect(t2.toString()).toEqual('[object RationalTime]')
})

test('equality', () => {
    const t1 = new lib.RationalTime(30.1)
    expect(t1).toEqual(t1)

    const t2 = new lib.RationalTime(30.1)
    expect(t2).not.toBe(t1)
    expect(t2).toEqual(t1)
});

test('inequality', () => {
    const t1 = new lib.RationalTime(30.2)
    const t2 = new lib.RationalTime(33.2)
    expect(t1.equal(t2)).toBeFalsy()
    expect(t1.notEqual(t2)).toBeTruthy()
    expect(t1 != t2).toBe(true)

    // console.log(t1.toPrimitive('number'), t1.valueOf())
    const t3 = new lib.RationalTime(30.2)
    // console.log(t3.toPrimitive('number'), t3.valueOf())
    expect(t1.equal(t3)).toBe(true)
    expect(t1.notEqual(t3)).toBeFalsy()
})

test('comparison', () => {
    const t1 = new lib.RationalTime(15.2)
    let t2 = new lib.RationalTime(15.6)
    console.log(t1.valueOf())
    expect(t1.lessThan(t2)).toBeTruthy()
    expect(t1.lessThanOrEqual(t2)).toBeTruthy()
    expect(t1.greaterThan(t1)).toBeFalsy()
    expect(t1.greaterThanOrEqual(t2)).toBeFalsy()

    // Ensure the equality case of the comparisons works correctly
    const t3 = new lib.RationalTime(30.4, 2)
    expect(t1.lessThanOrEqual(t3)).toBeTruthy()
    expect(t1.greaterThanOrEqual(t3)).toBeTruthy()
    expect(t3.lessThanOrEqual(t1)).toBeTruthy()
    expect(t3.greaterThanOrEqual(t1)).toBeTruthy()

    // test implicit base conversion
    t2 = new lib.RationalTime(15.6, 48)
    expect(t1.greaterThan(t2)).toBeTruthy()
    expect(t1.greaterThanOrEqual(t2)).toBeTruthy()
    expect(t1.lessThan(t2)).toBeFalsy()
    expect(t1.lessThanOrEqual(t2)).toBeFalsy()
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
