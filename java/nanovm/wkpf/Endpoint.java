package nanovm.wkpf;

public class Endpoint {
    public byte nodeId;
    public byte portNumber;
    public Endpoint(byte nodeId, byte portNumber) {
        this.nodeId = nodeId;
        this.portNumber = portNumber;
    }
}