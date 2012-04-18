package nanovm.wkpf;

public class Endpoint {
    public byte nodeId;
    public byte portNumber;
    public short profileId; // Actually not necessary, but added to check if a node doing a remote update is really writing to the profile it wants to write to.
    public Endpoint(byte nodeId, byte portNumber, short profileId) {
        this.nodeId = nodeId;
        this.portNumber = portNumber;
        this.profileId = profileId;
    }
}