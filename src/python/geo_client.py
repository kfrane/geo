""" TODO: Thread safety?  """
class GeoClient(object):
    def update(self, car_id, x, y):
        """Updates the location of car with car_id, with new coordinates."""
        raise Exception('Must implement this method')

    def query_polygon(self, vertices):
        """Returns all the cars inside a polygon defined by its vertices.
           Vertices is a list of tuples (xi, yi)."""
        raise Exception('Must implement this method')

    def query_knn(self, x, y, k):
        """Returns k nearest cars from point (x, y)."""
        raise Exception('Must implement this method')
