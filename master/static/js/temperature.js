// vim: ts=4 sw=4
function Temperature()
{
	Block.prototype.init.call(this);	
	this.type='Temperature';
	this.addSignal( new Signal('current'));
	this.addAction( new Action('refresh'));
	this.addAction( new Action('period'));
}

Temperature.prototype = new Block();
Temperature.prototype.constructor = Temperature;

Temperature.prototype.getCurrent=function() {
	// Get data from the master here
	return 0;
}
Temperature.prototype.getPeriod=function() {
	// Get data from the master here
	return 0;
}

Block.register('temperature',Temperature);
