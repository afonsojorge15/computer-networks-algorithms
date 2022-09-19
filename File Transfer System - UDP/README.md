# File Transfer System

For project description see [project statement](https://github.com/afonsojorge15/computer-networks-algorithms/blob/master/File%20Transfer%20System%20-%20UDP/project-routing-simulator.pdf)

## Report

Date: Sun 08 Nov 2020 05:53:24 PM WET  
Repo: git@git.rnl.tecnico.ulisboa.pt:RC-20-21/ist193680-proj1.git  
Commit: 7c7190139718d5072b6a7a4e50f83d708f073aca

### Build

- Found `Makefile`.
- Build succeeded.
- Found `file-sender`.
- Found `file-receiver`.

### Tests

| Test                      |  Result  |
| ------------------------- | :------: |
| Sending small text file   |    OK    |
| Sending binary file       |    OK    |
| Sending 500 byte file     |    OK    |
| Sending 1000 byte file    |    OK    |
| Stop & Wait. No Loss      |    OK    |
| Stop & Wait. Loss         |    OK    |
| Go Back N. No Loss        |    OK    |
| Go Back N. Loss           | **FAIL** |
| Selective Repeat. No Loss |    OK    |
| Selective Repeat. Loss    | **FAIL** |
| Message format            | **FAIL** |
| Triple loss.              | **FAIL** |
