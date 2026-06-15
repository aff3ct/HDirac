#!/usr/bin/env python3

"""
Regression test for HDirac CV-QKD simulations.

Runs HDirac on each test matrix, parses the output, loads the reference file,
and checks that FER values are within a tolerance band defined by the
statistical standard deviation:

    sigma = sqrt(FER * (1 - FER) / FRA)

Pass/fail logic (inspired by AFF3CT):
  - Noise points with FRA < --min-fra are skipped.
  - Each remaining point is individually validated (PASS / FAIL).
  - A test case is STRONG PASSED  if all validated points pass.
  - A test case is WEAK   PASSED  if pass_rate >= --weak-rate.
  - A test case is FAILED         otherwise.

Example Usage (called from .gitlab-ci.yml):
    ./ci/test-regression.py \
        --refs-path    ci/error_rate_references/LDPC/coset/householder_od2/sequence \
        --results-path test-regression-results \
        --build-path   build_linux_gcc_x64_avx2 \
        --binary-path  bin/HDirac \
        --sensibility  2.5 \
        --weak-rate    0.9 \
        --verbose      1
"""

from __future__ import annotations

import argparse
import re
import subprocess
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import List, Optional, Tuple

import numpy as np

# ---------------------------------------------------------------------------
# Data structures
# ---------------------------------------------------------------------------


@dataclass
class SimPoint:
    """One row of simulation / reference output."""

    ebn0_lin: float
    ebn0_db: float
    beta: float
    fra: int
    be: int
    fe: int
    ber: float
    fer: float

    @property
    def sigma_fer(self) -> float:
        """Statistical standard deviation on FER: sqrt(FER*(1-FER)/FRA)."""
        if self.fra <= 0:
            return float("inf")
        return float(np.sqrt(self.fer * (1.0 - self.fer) / self.fra))


@dataclass
class TestCase:
    """Maps one test-matrix to its reference file and CLI parameters."""

    matrix_path: str
    ref_filename: str
    channel: str
    decoder: str
    mode: str
    d: int
    max_error_frame: int
    max_frame: int
    beta_max: float
    beta_min: float
    beta_step: float
    extra_args: List[str] = field(default_factory=list)


# ---------------------------------------------------------------------------
# Test cases
# ---------------------------------------------------------------------------

TEST_CASES: List[TestCase] = [
    TestCase(
        matrix_path="ci/test_matrices/input/n204800_r0p09765625_cil_RA_SPPCOM_2.bsparse",
        ref_filename="n204800_r0p09765625_VC64_flooding_SPA.txt",
        channel="householder_od2",
        decoder="coset",
        mode="sequence",
        d=64,
        max_error_frame=100,
        max_frame=1000,
        beta_max=1.0,
        beta_min=0.90,
        beta_step=0.09,
    ),
]


# ---------------------------------------------------------------------------
# Parsing
# ---------------------------------------------------------------------------

_SEP = r"[\s|]+"
_FLOAT = r"([0-9eE+\-.]+)"

_DATA_RE = re.compile(
    r"^\s*"
    + _FLOAT
    + _SEP  # 0 ebn0_lin
    + _FLOAT
    + _SEP  # 1 ebn0_db
    + _FLOAT
    + _SEP  # 2 beta
    + _FLOAT
    + _SEP  # 3 fra
    + _FLOAT
    + _SEP  # 4 be
    + _FLOAT
    + _SEP  # 5 fe
    + _FLOAT
    + _SEP  # 6 ber
    + _FLOAT  # 7 fer   (trailing columns ignored)
)


def _parse_line(line: str) -> Optional[SimPoint]:
    stripped = line.strip()
    if not stripped or stripped.startswith("#"):
        return None
    m = _DATA_RE.match(stripped)
    if not m:
        return None
    g = m.groups()
    try:
        return SimPoint(
            ebn0_lin=float(g[0]),
            ebn0_db=float(g[1]),
            beta=float(g[2]),
            fra=int(float(g[3])),
            be=int(float(g[4])),
            fe=int(float(g[5])),
            ber=float(g[6]),
            fer=float(g[7]),
        )
    except (TypeError, ValueError):
        return None


def parse_lines(lines: List[str]) -> List[SimPoint]:
    pts: List[SimPoint] = []
    for ln in lines:
        pt = _parse_line(ln)
        if pt is not None:
            pts.append(pt)
    return pts


def parse_file(path: Path) -> List[SimPoint]:
    with open(str(path), "r", encoding="utf-8", errors="replace") as f:
        return parse_lines(f.readlines())


# ---------------------------------------------------------------------------
# Runner
# ---------------------------------------------------------------------------


def run_simulation(binary: Path, tc: TestCase) -> Tuple[int, str, str]:
    """Invoke HDirac and return (returncode, stdout, stderr)."""
    cmd = [
        str(binary),
        "--file",
        tc.matrix_path,
        "--channel",
        tc.channel,
        "--decoder",
        tc.decoder,
        "--mode",
        tc.mode,
        "--d",
        str(tc.d),
        "--max-error-frame",
        str(tc.max_error_frame),
        "--max-frame",
        str(tc.max_frame),
        "--beta-max",
        "{:.2f}".format(tc.beta_max),
        "--beta-min",
        "{:.2f}".format(tc.beta_min),
        "--beta-step",
        "{:.2f}".format(tc.beta_step),
    ] + tc.extra_args

    print("  $ {}".format(" ".join(cmd)))
    sys.stdout.flush()

    result = subprocess.run(cmd, capture_output=True, text=True)
    return result.returncode, result.stdout, result.stderr


# ---------------------------------------------------------------------------
# Comparison
# ---------------------------------------------------------------------------


def find_matching_ref(
    sim: SimPoint,
    refs: List[SimPoint],
    tol_db: float,
) -> Optional[SimPoint]:
    """Closest reference row by Eb/N0 (dB), within tol_db."""
    best: Optional[SimPoint] = None
    best_diff: float = float("inf")
    for r in refs:
        diff = abs(r.ebn0_db - sim.ebn0_db)
        if diff < best_diff and diff <= tol_db:
            best_diff = diff
            best = r
    return best


def compare_point(
    sim: SimPoint,
    ref: SimPoint,
    n_sigma: float,
) -> Tuple[bool, str]:
    """
    The tolerance is built from the *reference* FER and FRA:
        sigma = sqrt(FER_ref * (1 - FER_ref) / FRA_ref)
        tol   = n_sigma * sigma

    A point PASSES when |FER_sim - FER_ref| <= tol.

    EXCEPTION HANDLING FOR SATURATION:
    If ref.fer >= 0.99, the statistical sigma window shrinks drastically
    (or becomes 0.0 at exactly 1.0). Instead of forcing an exact match,
    we apply a fixed 1% engineering tolerance window.
    """
    sigma = ref.sigma_fer  # sqrt(p*(1-p)/n) on the reference
    fer_diff = abs(sim.fer - ref.fer)

    # If the reference is saturated at or near 100% error, apply standard 1% tolerance
    if ref.fer >= 0.99:
        tol = 0.01
        passed = fer_diff <= tol
    # Exact-match shortcut for pure zero-error edge cases (sigma == 0)
    elif sigma == 0.0:
        tol = 0.0
        passed = fer_diff == 0.0
    # Standard statistical comparison
    else:
        tol = n_sigma * sigma
        passed = fer_diff <= tol

    status = "PASS" if passed else "FAIL"

    msg = (
        "  [{status}] Eb/N0={ebn0:7.2f} dB  Beta={beta:6.2f}%"
        "  FER_sim={fs:.3e}  FER_ref={fr:.3e}"
        "  |diff|={d:.3e}  tol={t:.3e} ({n}σ, σ={s:.3e})"
    ).format(
        status=status,
        ebn0=sim.ebn0_db,
        beta=sim.beta,
        fs=sim.fer,
        fr=ref.fer,
        d=fer_diff,
        t=tol,
        n=n_sigma if ref.fer < 0.99 else 0.0,
        s=sigma,
    )
    return passed, msg


def compute_pass_rate(n_valid: int, n_total: int) -> float:
    """Fraction of validated points that passed."""
    if n_total == 0:
        return 0.0
    return float(n_valid) / float(n_total)


# ---------------------------------------------------------------------------
# Logging helper
# ---------------------------------------------------------------------------


def write_log(
    results_path: Path,
    stem: str,
    ref_file: str,
    args: argparse.Namespace,
    log_lines: List[str],
    matched: int,
    total: int,
) -> None:
    log_file = results_path / "{}_comparison.txt".format(stem)
    with open(str(log_file), "w") as f:
        f.write("Reference  : {}\n".format(ref_file))
        f.write("sensibility: {} sigma\n".format(args.sensibility))
        f.write("weak_rate  : {}\n".format(args.weak_rate))
        f.write("matched    : {}/{}\n\n".format(matched, total))
        f.write("\n".join(log_lines) + "\n")


def vlog(msg: str, min_level: int, verbosity: int) -> None:
    if verbosity >= min_level:
        print(msg)
        sys.stdout.flush()


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(
        description="HDirac regression test",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    p.add_argument(
        "--refs-path", required=True, help="Directory containing reference .txt files"
    )
    p.add_argument(
        "--results-path",
        default="test-regression-results",
        help="Directory where results and logs are written",
    )
    p.add_argument(
        "--build-path",
        required=True,
        help="Build directory (e.g. build_linux_gcc_x64_avx2)",
    )
    p.add_argument(
        "--binary-path",
        default="bin/HDirac",
        help="Path to binary relative to --build-path",
    )
    p.add_argument(
        "--sensibility",
        type=float,
        default=3.0,
        help="Number of standard deviations used as FER tolerance",
    )
    p.add_argument(
        "--weak-rate",
        type=float,
        default=0.9,
        help="Minimum pass-rate to be WEAK PASSED (0.0–1.0)",
    )
    p.add_argument(
        "--ebn0-tol-db",
        type=float,
        default=0.02,
        help="Max Eb/N0 gap (dB) for matching sim rows to reference rows",
    )
    p.add_argument(
        "--min-fra",
        type=int,
        default=50,
        help="Minimum number of frames required to validate a noise point "
        "(points below this are skipped as unconverged, --min-fe)",
    )
    p.add_argument(
        "--verbose",
        type=int,
        default=1,
        help="Verbosity level: 0=quiet, 1=normal, 2=debug",
    )
    return p.parse_args()


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------


def main() -> int:
    args = parse_args()
    refs_path = Path(args.refs_path)
    results_path = Path(args.results_path)
    binary = Path(args.build_path) / args.binary_path
    verbosity = args.verbose

    results_path.mkdir(parents=True, exist_ok=True)

    if not binary.exists():
        print("[ERROR] Binary not found: {}".format(binary), file=sys.stderr)
        return 1

    # ---- counters --------
    n_tests = len(TEST_CASES)
    n_strong_pass = 0
    n_weak_pass = 0
    n_failed = 0
    fail_names: List[str] = []

    for tc_idx, tc in enumerate(TEST_CASES, start=1):
        ref_file = refs_path / tc.ref_filename
        stem = Path(tc.matrix_path).stem

        vlog("\n" + "=" * 70, 1, verbosity)
        vlog("Test {}/{} : {}".format(tc_idx, n_tests, tc.matrix_path), 1, verbosity)
        vlog("Ref        : {}".format(ref_file), 1, verbosity)
        vlog("=" * 70, 1, verbosity)

        # ---- reference -----------------------------------------------------
        if not ref_file.exists():
            print(
                "[ERROR] Reference file not found: {}".format(ref_file), file=sys.stderr
            )
            n_failed += 1
            fail_names.append(tc.matrix_path)
            write_log(
                results_path,
                stem,
                str(ref_file),
                args,
                ["[ERROR] Reference file not found"],
                0,
                0,
            )
            continue

        refs = parse_file(ref_file)
        vlog("  Reference rows loaded: {}".format(len(refs)), 1, verbosity)
        if verbosity >= 2:
            for r in refs:
                vlog(
                    "    ref Eb/N0={:.2f} dB  Beta={:.2f}%  FER={:.3e}  FRA={}".format(
                        r.ebn0_db, r.beta, r.fer, r.fra
                    ),
                    2,
                    verbosity,
                )

        if not refs:
            print(
                "[ERROR] No data rows found in reference file: {}".format(ref_file),
                file=sys.stderr,
            )
            n_failed += 1
            fail_names.append(tc.matrix_path)
            write_log(
                results_path,
                stem,
                str(ref_file),
                args,
                ["[ERROR] No data rows in reference file"],
                0,
                0,
            )
            continue

        # ---- simulation ----------------------------------------------------
        rc, stdout, stderr = run_simulation(binary, tc)

        out_file = results_path / "{}_output.txt".format(stem)
        with open(str(out_file), "w") as f:
            f.write(stdout)
        vlog("  Raw output -> {}".format(out_file), 2, verbosity)

        if stderr.strip():
            err_file = results_path / "{}_stderr.txt".format(stem)
            with open(str(err_file), "w") as f:
                f.write(stderr)
            vlog("  Stderr     -> {}".format(err_file), 2, verbosity)

        if rc != 0:
            print("[ERROR] Simulation exited with code {}".format(rc), file=sys.stderr)
            print(stderr, file=sys.stderr)
            n_failed += 1
            fail_names.append(tc.matrix_path)
            write_log(
                results_path,
                stem,
                str(ref_file),
                args,
                ["[ERROR] Simulation failed (exit {})".format(rc), stderr],
                0,
                0,
            )
            continue

        if stderr.strip():
            vlog("---- Warning message(s):", 1, verbosity)
            vlog(stderr, 1, verbosity)

        # ---- parse ---------------------------------------------------------
        sim_points = parse_lines(stdout.splitlines())
        vlog("  Simulation rows parsed: {}".format(len(sim_points)), 1, verbosity)
        if verbosity >= 2:
            for s in sim_points:
                vlog(
                    "    sim Eb/N0={:.2f} dB  Beta={:.2f}%  FER={:.3e}  FRA={}".format(
                        s.ebn0_db, s.beta, s.fer, s.fra
                    ),
                    2,
                    verbosity,
                )

        if not sim_points:
            print("[ERROR] No data rows found in simulation output.", file=sys.stderr)
            n_failed += 1
            fail_names.append(tc.matrix_path)
            write_log(
                results_path,
                stem,
                str(ref_file),
                args,
                ["[ERROR] No sim rows parsed"],
                0,
                0,
            )
            continue

        # ---- compare ----------------------------------------
        n_valid: int = 0  # points that PASSED
        matched: int = 0  # points that had a reference match
        unmatched_ebn0s: List[float] = []
        log_lines: List[str] = []

        for sp in sim_points:
            # Skip points that didn't accumulate enough frames to be meaningful
            if sp.fra < args.min_fra:
                log_lines.append(
                    "  [SKIP] Eb/N0={:.2f} dB  Beta={:.2f}%"
                    "  FRA={} < --min-fra={} (unconverged)".format(
                        sp.ebn0_db, sp.beta, sp.fra, args.min_fra
                    )
                )
                continue

            ref_pt = find_matching_ref(sp, refs, tol_db=args.ebn0_tol_db)
            if ref_pt is None:
                unmatched_ebn0s.append(sp.ebn0_db)
                log_lines.append(
                    "  [SKIP] Eb/N0={:.2f} dB  Beta={:.2f}%"
                    " — no matching reference row".format(sp.ebn0_db, sp.beta)
                )
                continue

            matched += 1
            passed, msg = compare_point(sp, ref_pt, args.sensibility)
            log_lines.append(msg)
            if passed:
                n_valid += 1

        for line in log_lines:
            vlog(line, 1, verbosity)

        if unmatched_ebn0s:
            vlog(
                "  [WARN] {n} sim point(s) had no reference match "
                "(Eb/N0 dB): {v}".format(
                    n=len(unmatched_ebn0s),
                    v=["{:.2f}".format(x) for x in unmatched_ebn0s],
                ),
                1,
                verbosity,
            )

        write_log(
            results_path, stem, str(ref_file), args, log_lines, matched, len(sim_points)
        )

        if matched == 0:
            print(
                "  [ERROR] No simulation points matched any reference row.",
                file=sys.stderr,
            )
            n_failed += 1
            fail_names.append(tc.matrix_path)
            continue

        pass_rate = compute_pass_rate(n_valid, matched)

        if pass_rate == 1.0:
            vlog(
                "\n  => Test case STRONG PASSED"
                "  (matched {}/{} points, pass rate {:.2f})".format(
                    matched, len(sim_points), pass_rate
                ),
                1,
                verbosity,
            )
            n_strong_pass += 1

        elif pass_rate >= args.weak_rate:
            vlog(
                "\n  => Test case WEAK PASSED"
                "  (matched {}/{} points, pass rate {:.2f})".format(
                    matched, len(sim_points), pass_rate
                ),
                1,
                verbosity,
            )
            n_weak_pass += 1

        else:
            vlog(
                "\n  => Test case FAILED"
                "  (matched {}/{} points, pass rate {:.2f})".format(
                    matched, len(sim_points), pass_rate
                ),
                1,
                verbosity,
            )
            n_failed += 1
            fail_names.append(tc.matrix_path)

    # ---- global summary ---------------------
    vlog("\n" + "=" * 70, 1, verbosity)
    vlog(
        "# {} tests executed: {} strong passed, {} weak passed, {} failed".format(
            n_tests, n_strong_pass, n_weak_pass, n_failed
        ),
        1,
        verbosity,
    )

    if n_failed == 0:
        vlog("\n# (II) All the tests PASSED !", 1, verbosity)
        return 0

    vlog("\n# (II) FAILED tests:", 1, verbosity)
    for name in fail_names:
        vlog("  - {}".format(name), 1, verbosity)
    return 1


if __name__ == "__main__":
    sys.exit(main())
