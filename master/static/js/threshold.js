// vim: ts=4 sw=4
function Threshold()
{
	Block.prototype.init.call(this);	
	this.type='Threshold';
	this.addSignal( new Signal('output'));
	this.addAction( new Action('threshold'));
	this.addAction( new Action('value'));
}

Threshold.prototype = new Block();
Threshold.prototype.constructor = Threshold;
Block.register('Threshold',Threshold);
