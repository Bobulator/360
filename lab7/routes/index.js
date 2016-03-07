var fs = require('fs');
var request = require('request');
var express = require('express');
var router = express.Router();

var dir = 'testDir/';

router.get('/', function(req, res) {
  res.sendFile('weather.html', {root: 'public'});
});

router.get('/test1.html', function(req, res) {
  res.sendFile(dir + 'test1.html');
});

router.get('/test2.txt', function(req, res) {
  res.sendFile(dir + 'test2.txt');
});

router.get('/test3.gif', function(req, res) {
  res.sendFile(dir + 'test3.gif');
});

router.get('/test4.jpg', function(req, res) {
  res.sendFile(dir + 'test4.jpg');
});

router.get('/HasIndex/', function(req, res) {
  res.sendFile(dir + 'HasIndex/index.html');
});

router.get('/getcity', function(req, res, next) {
  console.log('In getcity route');
  var prefixReg = new RegExp("^" + req.query.q);
  console.log(prefixReg);
  fs.readFile(__dirname + '/cities.dat.txt',function(err,data) {
  if(err) throw err;
    var cities = data.toString().split("\n");
    var jsonresult = [];
	for(var i = 0; i < cities.length; i++) {
      var result = cities[i].search(prefixReg);
	  if (prefixReg == "/^undefined/" || result != -1) {
        jsonresult.push({city:cities[i]});
	  }
	}
	res.status(200).json(jsonresult);
  }) 
});

var wgurl = "https://api.wunderground.com/api/82975b43eac78dd6/geolookup/conditions/q/Utah/";
router.get('/getweatherdata', function(req, res) {
  console.log('In getweatherdata route');
  console.log(wgurl + req.query.q + ".json");
  request(wgurl + req.query.q + ".json").pipe(res);
});

router.get('/getwiki', function(req, res) {
  console.log('In getwiki route');
  var wikiurl = "https://en.wikipedia.org/w/api.php?format=json&action=query&prop=extracts&exintro=&explaintext=&titles=" + req.query.q + ",%20Utah";
  console.log(wikiurl);
  request(wikiurl).pipe(res);
});
module.exports = router;
