# Network Routing Protocols

For project description see [project statement](https://github.com/afonsojorge15/computer-networks-algorithms/blob/master/Network%20Routing%20Protocols/project-reliable-xfer.pdf)

## Report

Date: Sun 30 Jan 2022 06:11:25 PM WET  
Repo: git@git.rnl.tecnico.ulisboa.pt:RC-21-22/ist193680-proj2.git  
Commit: bb9f50475ad4a334c6a36a96b75bd482d04dd7ce

### Build

- Build succeeded.
- Found `dv-simulator`.
- Found `dvrpp-simulator`.
- Found `pv-simulator`.
- Found `ls-simulator`.

### Tests

| Topology              | Distance Vector | Distance Vector with Reverse Path Poisoning | Path Vector | Link State |
| --------------------- | :-------------: | :-----------------------------------------: | :---------: | :--------: |
| linear-3.net          |       OK        |                     OK                      |     OK      |  **FAIL**  |
| count-to-infinity.net |       OK        |                     OK                      |     OK      |  **FAIL**  |
| diamond.net           |       OK        |                     OK                      |     OK      |  **FAIL**  |
