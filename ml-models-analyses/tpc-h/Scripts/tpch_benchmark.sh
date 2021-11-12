#!/bin/bash

#
# Copyright (c) 2019- Ibrahim Umit Akgun
# Copyright (c) 2021- Andrew Burford
# Copyright (c) 2021- Mike McNeill
# Copyright (c) 2021- Michael Arkhangelskiy
# Copyright (c) 2020-2021 Aadil Shaikh
# Copyright (c) 2020-2021 Lukas Velikov
#
# You can redistribute it and/or modify it under the terms of the Apache License, 
# Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0).
#

# Revised version of TPC-H benchmarking script
# Cun run queries n times instead of just once

tpchDir="$(realpath . || exit $?)"
dbgenDir=""
queriesDir=""
mysqlDir=""
DBName="tpch"
outputDir=""
device=""
mysqlPassword="password"
numRuns=1
dataDefintionPath=""
secondaryLoadPath=""

# Uncomment for MySQL HeatWave TPC-H
dataDefinitionFilename="create_tables.sql"
secondaryLoadFilename="secondary_load.sql"

# Uncomment for Default TPC-H
# dataDefinitionFilename="dss.ddl"
# secondaryLoadFilename="dss.ri"

function runcmd
{
    echo "CMD: $@"
    sleep 0.2
    $@
    ret=$?
    if test $ret -ne 0 ; then
	exit $ret
    fi
}

# Run as --tpch-dir <dir> --setup-database
function setupDatabase
{
    echo "Dropping database ${DBName} if it already exists"
    mysql -u root -p"${mysqlPassword}" -e "DROP DATABASE IF EXISTS ${DBName}"
    echo "Creating database ${DBName}"
#    mysql -u root -p"${mysqlPassword}" -e "CREATE DATABASE ${DBName}"
    echo "Importing data definitions from ${dataDefinitionPath} to create tables"
    mysql -u root -p"${mysqlPassword}" < "${dataDefinitionPath}"
    for table in customer lineitem nation orders part partsupp region supplier; do
        echo "Loading ${dbgenDir}/${table}.tbl into table ${table}"
        mysql --local-infile=1 -u root -p"${mysqlPassword}" "${DBName}" -e "LOAD DATA LOCAL INFILE '${dbgenDir}/${table}.tbl' INTO TABLE ${table^^} FIELDS TERMINATED BY '|';"
    done
    echo "Altering table based on ${secondaryLoadPath}"
    mysql -u root -p"${mysqlPassword}" "${DBName}" < "${secondaryLoadPath}"
}

# Creates queries, but they're incompatible with MySQL
# Modifications to these queries would be needed
function createQueries
{
    runcmd cd "${dbgenDir}"
    runcmd cp dists.dss "${queriesDir}"
    runcmd cp qgen "${queriesDir}"
    runcmd cd "${queriesDir}"
    runcmd rm -f query-*.sql
    for i in {1..22}; do
        echo "Generating query-${i}.sql"
        ./qgen "${i}" > "query-${i}.sql"
    done
}

# ./tpch_benchmark --tpch-dir <d> --output-dir <d> --device <d> --runs-per-query <n> --execute-queries
function executeQueries
{
    runcmd cd "${queriesDir}"
    echo "Saving results to ${outputDir}"
    for queryNum in 18 21; do
	for ra in 8 16 32 64 128 256 512 1024 2048 4096; do
	    echo "Setting readahead of ${device} to ${ra}"
	    blockdev --setra $ra $device
            for runNum in $(seq 1 ${numRuns}); do
		free && sync && echo 3 > /proc/sys/vm/drop_caches && free
		outputFile="${outputDir}/query-${queryNum}-ra-${ra}-run-${runNum}-results.txt"
		echo "Executing and timing query ${queryNum} with readahead ${ra} run ${runNum}"
		/usr/bin/time -v mysql -u root -p"${mysqlPassword}" "${DBName}" < "hq${queryNum}.sql" &> "${outputFile}"
            done
	    service mysqld restart
	done
    done
}

# ./tpch_benchmark --tpch-dir <d> --output-dir <d> --device <d> --runs-per-query <n> --execute-baseline
function executeBaseline
{
    runcmd cd "${queriesDir}"
    echo "Saving results to ${outputDir}"
    ra=256
    for queryNum in 1 6 9 11 13 14 15 22; do
	echo "Setting readahead of ${device} to ${ra}"
	blockdev --setra $ra $device
        for runNum in $(seq 1 ${numRuns}); do
	    free && sync && echo 3 > /proc/sys/vm/drop_caches && free
	    outputFile="${outputDir}/query-${queryNum}-baseline-run-${runNum}-results.txt"
	    echo "Executing and timing query ${queryNum} with readahead ${ra} run ${runNum}"
	    /usr/bin/time -v mysql -u root -p"${mysqlPassword}" "${DBName}" < "hq${queryNum}.sql" &> "${outputFile}"
            echo "Result saved in ${outputFile}"
	done
        service mysqld restart
    done
}

# ./tpch_benchmark --tpch-dir <d> --output-dir <d> --device <d> --runs-per-query <n> --execute-kml
function executeKml
{
    defaultRA=256
    kmlModule="/home/kml/build/kml.ko"
    readaheadModule="/home/kml/kernel-interfaces/readahead/readahead.ko"
    runcmd cd "${queriesDir}"
    echo "Saving results to ${outputDir}"
    for queryNum in 1 6 9 11 13 14 15 22; do
        for runNum in $(seq 1 ${numRuns}); do
            echo "Setting readahead of ${device} to default value of ${defaultRA}"
            blockdev --setra "${defaultRA}" "${device}"
            free && sync && echo 3 > /proc/sys/vm/drop_caches && free
            echo "Inserting kml and readahead kernel modules"
            insmod "${kmlModule}" && insmod "${readaheadModule}"
            outputFile="${outputDir}/query-${queryNum}-kml-run-${runNum}-results.txt"
            echo "Executing and timing query ${queryNum} with readahead ${ra} run ${runNum}"
            /usr/bin/time -v mysql -u root -p"${mysqlPassword}" "${DBName}" < "hq${queryNum}.sql" &> "${outputFile}"
            echo "Result saved in ${outputFile}"
            echo "Removing readahead and kml kernel modules"
            rmmod readahead kml
            echo "Restarting mysqld service"
            service mysqld restart
        done
    done
}

while [[ $# -gt 0 ]]; do
    key="$1"
    case "${key}" in
        --tpch-dir)
            shift 
            tpchDir="$(realpath "$1" || exit $?)"
            dbgenDir="${tpchDir}/dbgen"
            dataDefinitionPath="${dbgenDir}/${dataDefinitionFilename}"
	        secondaryLoadPath="${dbgenDir}/${secondaryLoadFilename}"
	        queriesDir="${dbgenDir}/queries"
            shift
            ;;
        --setup-database)
            setupDatabase
            exit
            ;;
        --create-queries)
            createQueries
            exit
            ;;
        --execute-queries)
            executeQueries
            exit
            ;;
        --execute-baseline)
            executeBaseline
            exit
            ;;
        --execute-kml)
            executeKml
            exit
            ;;
	    --runs-per-query)
            shift
            numRuns="$1"
            shift
            ;;
        --output-dir)
            shift 
            outputDir="$(realpath "$1" || exit $?)"
            shift
            ;;
        --device)
            shift
            device="$1"
            shift
            ;;
        *)
            shift # past argument
            ;;
    esac
done
