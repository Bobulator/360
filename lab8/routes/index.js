var express = require('express');
var router = express.Router();
var mongoose = require('mongoose');

/* Set up mongoose in order to connect to mongo database */
//Connects to a mongo database called "commentDB"
mongoose.connect('mongodb://localhost/commentDB');

//Defines the Schema for this database
var commentSchema = mongoose.Schema({
  Name: String,
  Comment: String
});

//Makes an object from that schema as a modela
var Comment = mongoose.model('Comment', commentSchema);

//Saves the connection as a variable to use
var db = mongoose.connection;

//Checks for connection errors
db.on('error', console.error.bind(console, 'connection error:'));

//Lets us know when we're connected
db.once('open', function() {
  console.log('Connected');
});

router.post('/comment', function(req, res, next) {
  console.log("POST comment route"); //[1]
  console.log(req.body);
  
  var newcomment = new Comment(req.body); //[3]
  console.log(newcomment); //[3]
  newcomment.save(function(err, post) { //[4]
    if (err) return console.error(err);
    console.log(post);
    res.sendStatus(200);
  });
});

/* GET comments from database */
router.get('/comment', function(req, res, next) {
  console.log("In the GET route");
  
  //Calls the find() method on your database
  Comment.find(function(err, commentList) {
    
    //If there's an error, print it out
    if (err) return console.error(err);
    else {
      //Otherwise console log the comments you found
      console.log(commentList);
      res.json(commentList);
      
    }
  })
});

module.exports = router;
