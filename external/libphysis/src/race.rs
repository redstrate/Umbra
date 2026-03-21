// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

use physis::race::{Gender, Race, Tribe, get_race_id, get_supported_tribes};

#[unsafe(no_mangle)]
pub extern "C" fn physis_get_race_code(race: Race, tribe: Tribe, gender: Gender) -> i32 {
    get_race_id(race, tribe, gender).unwrap()
}

#[repr(C)]
pub struct physis_SupportedTribes {
    subraces: [Tribe; 2],
}

#[unsafe(no_mangle)]
pub extern "C" fn physis_get_supported_tribes(race: Race) -> physis_SupportedTribes {
    physis_SupportedTribes {
        subraces: get_supported_tribes(race),
    }
}
