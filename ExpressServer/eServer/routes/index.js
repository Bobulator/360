var express = require('express');
var router = express.Router();
var fs = require('fs');

/* GET home page. */
router.get('/', function(req, res, next) {
  res.sendFile('home.html', { root: 'public' });
});
router.get('/getcity', function(req, res) {
  console.log(req.query);
  var queryReg = new RegExp("^" + req.query.q);
  fs.readFile(__dirname + '/cities.dat.txt', function(err, data) {
    if(err) throw err;
	var cities = data.toString().split('\n');
	var jsonResult = [];
	for (var i = 0; i < cities.length; i++) {
	  var result = cities[i].search(queryReg);
	  if (result != -1) {
	  	console.log(cities[i]);
		jsonResult.push({ city : cities[i] }); 
	  }
	}
	res.status(200).json(jsonResult);
  });
});

module.exports = router;
