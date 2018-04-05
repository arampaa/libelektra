@INCLUDE_COMMON@

echo
echo ELEKTRA CHECK FORMATTING
echo

command -v git >/dev/null 2>&1 || { echo "git command needed for this test, skipping" >&2; exit @SKIP_RETURN_CODE@; }
command -v clang-format-5.0 >/dev/null 2>&1 || command -v clang-format >/dev/null 2>&1 || { echo "clang-format command needed for this test, skipping" >&2; exit @SKIP_RETURN_CODE@; }

cd "@CMAKE_SOURCE_DIR@"

if ! git diff --exit-code
then
	echo "Seems like source is already modified, aborting test" >&2
	exit 0
fi

scripts/reformat-source

git diff --exit-code
succeed_if "Please commit the reformatting changes before pushing"

end_script
