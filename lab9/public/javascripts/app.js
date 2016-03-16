angular.module('comment', [])
.controller('MainCtrl', [
  '$scope',
  function($scope){
    $scope.test = 'Hello world!';
  }
  $scope.comments = [
      'Comment 1',
      'Comment 2',
      'Comment 3',
      'Comment 4',
      'Comment 5'
    ];
]);
