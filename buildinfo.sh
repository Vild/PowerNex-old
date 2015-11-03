#!/bin/bash
cat > includes/buildinfo.h << EOF
#ifndef BUILDINFO_H_
#define BUILDINFO_H_

extern const char * build_git_version;
extern const char * build_git_sha;
extern const char * build_time;

#endif
EOF

cat > $1 << EOF
const char * build_git_version = "$(git describe --abbrev=4 --dirty --always --tags)";
const char * build_git_sha = "$(git rev-parse HEAD)";
const char * build_time = "$(date)";

EOF
