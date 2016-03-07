var express = require('express');
var router = express.Router();

router.post('/comment', function(req, res, next) {
  console.log("POST comment route"); //[1]
  console.log(req.body);
  res.sendStatus(200);
});


module.exports = router;
