// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

extern crate cbindgen;

use std::env;
use std::fs;
use std::fs::File;
use std::io::BufRead;
use std::io::BufReader;
use std::io::BufWriter;
use std::io::Write;

use cbindgen::Language;

fn main() {
    let crate_dir = env::var("CARGO_MANIFEST_DIR").unwrap();

    cbindgen::Builder::new()
        .with_crate(crate_dir.as_str())
        .with_parse_deps(true)
        .with_parse_include(&["physis"])
        .with_language(Language::C)
        .generate()
        .expect("Unable to generate C bindings")
        .write_to_file("target/public/physis.h");

    cbindgen::Builder::new()
        .with_crate(crate_dir.as_str())
        .with_parse_deps(true)
        .with_parse_include(&["physis"])
        .with_language(Language::Cxx)
        .with_pragma_once(true)
        .generate()
        .expect("Unable to generate C++ bindings")
        .write_to_file("target/public/physis.hpp");

    // cbindgen always includes <ostream> and <new> even if they aren't used
    // some downstream projects like PhysisSharp need to have a cleaner file
    {
        let file: File = File::open("target/public/physis.hpp").unwrap();
        let out_file: File = File::create("target/public/physis.hpp.temp").unwrap();

        let reader = BufReader::new(&file);
        let mut writer = BufWriter::new(&out_file);

        for line in reader.lines() {
            let line = line.as_ref().unwrap();
            if !line.contains("#include <ostream>") && !line.contains("#include <new>") {
                writeln!(writer, "{}", line).expect("Failed to replace include");
            }
        }
    }
    fs::rename("target/public/physis.hpp.temp", "target/public/physis.hpp").unwrap();
}
